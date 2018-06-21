#include "archivator.h"
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>


void archivator::compress(const std::string &dir, const std::string &destination)
{
	std::map<fs::path, fs::path> files;
	fs::path dir_path;
	try {
		dir_path = fs::canonical(fs::path(dir));
		if (fs::is_directory(dir_path)) {
			for (auto i = fs::recursive_directory_iterator(dir_path); i != fs::recursive_directory_iterator(); ++i) {
				fs::path file = *i;
				if (fs::is_regular_file(file)) {
					// find out where the two paths diverge - boost::filesystem::relative() is too new now to use it
					fs::path::const_iterator itr_dir = dir_path.begin();
					fs::path::const_iterator itr_file = file.begin();
					while (*itr_dir == *itr_file && itr_dir != dir_path.end()) {
						++itr_dir;
						++itr_file;
					}
					// result is what is left in second path (it's relative path to the first one)
					fs::path result;
					while (itr_file != file.end()) {
						result /= *itr_file;
						itr_file++;
					}
					files.insert({file, result});
				}
			}
		} else {
			throw archive_exception("Given path '" + dir + "' is not directory.");
		}
	} catch (fs::filesystem_error &e) {
		throw archive_exception(e.what());
	}

	std::unique_ptr<archive, decltype(&archive_write_free)> a = {archive_write_new(), archive_write_free};
	if (a == nullptr) { throw archive_exception("Cannot create destination archive."); }
	if (archive_write_set_format_zip(a.get()) != ARCHIVE_OK) {
		throw archive_exception("Cannot set ZIP format on destination archive.");
	}
	if (archive_write_open_filename(a.get(), destination.c_str()) != ARCHIVE_OK) {
		throw archive_exception("Cannot open destination archive.");
	}

	for (auto &file : files) {
		std::unique_ptr<archive_entry, decltype(&archive_entry_free)> entry = {archive_entry_new(), archive_entry_free};

		archive_entry_set_pathname(entry.get(), (fs::path(destination).stem() / file.second).string().c_str());
		archive_entry_set_size(entry.get(), fs::file_size(file.first));
		archive_entry_set_mtime(entry.get(), fs::last_write_time(file.first), 0); // 0 nanoseconds
		archive_entry_set_filetype(entry.get(), AE_IFREG);
		archive_entry_set_perm(entry.get(), 0644);

		int r = archive_write_header(a.get(), entry.get());
		if (r < ARCHIVE_OK) { throw archive_exception(archive_error_string(a.get())); }

		std::ifstream ifs((file.first).string(), std::ios::in | std::ios::binary);
		if (ifs.is_open()) {
			// read data by small blocks to avoid memory overfill on possibly large files
			char buff[4096];

			while (true) {
				ifs.read(buff, sizeof(buff));

				auto read_len = ifs.gcount();
				if (ifs.eof() && read_len == 0) {
					break;
				} else if (read_len <= 0) {
					throw archive_exception("Error reading input file.");
				}

				r = archive_write_data(a.get(), buff, static_cast<size_t>(ifs.gcount()));
				if (r < ARCHIVE_OK) { throw archive_exception(archive_error_string(a.get())); }
			}
		} else {
			throw archive_exception("Cannot open file " + (file.first).string() + " for reading.");
		}
	}

	archive_write_close(a.get());
}


void archivator::decompress(const std::string &filename, const std::string &destination)
{
	if (!fs::is_directory(destination)) {
		throw archive_exception("Destination '" + destination + "' is not a directory. Cannot decompress archive.");
	}
	if (!fs::is_regular_file(filename)) {
		throw archive_exception("Source archive '" + filename + "' not exists or is not a regular file.");
	}

	// Select which attributes we want to restore.
	int flags;
	flags = ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_FFLAGS;
	// Don't allow ".." in any path within archive
	flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;

	std::unique_ptr<archive, decltype(&archive_write_free)> a = {archive_write_new(), archive_write_free};
	if (a == nullptr) { throw archive_exception("Cannot create source archive."); }
	if (archive_read_support_format_all(a.get()) != ARCHIVE_OK) {
		throw archive_exception("Cannot set formats for source archive.");
	}
	if (archive_read_support_filter_all(a.get()) != ARCHIVE_OK) {
		throw archive_exception("Cannot set compression methods for source archive.");
	}

	std::unique_ptr<archive, decltype(&archive_write_free)> ext = {archive_write_disk_new(), archive_write_free};
	if (ext == nullptr) { throw archive_exception("Cannot allocate archive entry."); }
	if (archive_write_disk_set_options(ext.get(), flags) != ARCHIVE_OK) {
		throw archive_exception("Cannot set options for writing to disk.");
	}
	if (archive_write_disk_set_standard_lookup(ext.get()) != ARCHIVE_OK) {
		throw archive_exception("Cannot set lookup for writing to disk.");
	}

	int r = archive_read_open_filename(a.get(), filename.c_str(), 10240);
	if (r < ARCHIVE_OK) { throw archive_exception("Cannot open source archive."); }

	while (true) {
		archive_entry *entry;
		r = archive_read_next_header(a.get(), &entry);
		if (r == ARCHIVE_EOF) { break; }
		if (r < ARCHIVE_OK) { throw archive_exception(archive_error_string(a.get())); }

		const char *current_file = archive_entry_pathname(entry);
		const std::string full_path = (fs::path(destination) / current_file).string();
		archive_entry_set_pathname(entry, full_path.c_str());
		auto filetype = archive_entry_filetype(entry);
		if (filetype == AE_IFREG) {
			archive_entry_set_perm(entry, 0644);
		} else if (filetype == AE_IFDIR) {
			archive_entry_set_perm(entry, 0755);
		} else {
			throw archive_exception("Unsupported archive entry filetype.");
		}

		r = archive_write_header(ext.get(), entry);
		if (r < ARCHIVE_OK) { throw archive_exception(archive_error_string(ext.get())); }

		if (archive_entry_size(entry) > 0) { copy_data(a.get(), ext.get()); }

		r = archive_write_finish_entry(ext.get());
		if (r < ARCHIVE_OK) { throw archive_exception(archive_error_string(ext.get())); }
	}

	archive_read_close(a.get());
	archive_write_close(ext.get());
}


void archivator::copy_data(archive *ar, archive *aw)
{
	int64_t r;
	const void *buff;
	size_t size;
	int64_t offset;

	while (true) {
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF) { break; }
		if (r < ARCHIVE_OK) { throw archive_exception(archive_error_string(ar)); }
		r = archive_write_data_block(aw, buff, size, offset);
		if (r < ARCHIVE_OK) { throw archive_exception(archive_error_string(aw)); }
	}
}

#include "archivator.h"
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>


void archivator::compress(const std::string &dir, const std::string &destination)
{
	archive *a;
	archive_entry *entry;
	int r;

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

	a = archive_write_new();
	if (a == NULL) {
		throw archive_exception("Cannot create destination archive.");
	}
	if (archive_write_set_format_zip(a) != ARCHIVE_OK) {
		throw archive_exception("Cannot set ZIP format on destination archive.");
	}
	if (archive_write_open_filename(a, destination.c_str()) != ARCHIVE_OK) {
		throw archive_exception("Cannot open destination archive.");
	}

	for (auto &file : files) {
		entry = archive_entry_new();

		archive_entry_set_pathname(entry, (fs::path(destination).stem() / file.second).string().c_str());
		archive_entry_set_size(entry, fs::file_size(file.first));
		archive_entry_set_mtime(entry, fs::last_write_time(file.first), 0); // 0 nanoseconds
		archive_entry_set_filetype(entry, AE_IFREG);
		archive_entry_set_perm(entry, 0644);

		r = archive_write_header(a, entry);
		if (r < ARCHIVE_OK) {
			throw archive_exception(archive_error_string(a));
		}

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

				r = archive_write_data(a, buff, static_cast<size_t>(ifs.gcount()));
				if (r < ARCHIVE_OK) {
					throw archive_exception(archive_error_string(a));
				}
			}
		} else {
			throw archive_exception("Cannot open file " + (file.first).string() + " for reading.");
		}
		archive_entry_free(entry);
	}
	archive_write_close(a);
	archive_write_free(a);
}


void archivator::decompress(const std::string &filename, const std::string &destination)
{
	archive *a;
	archive *ext;
	archive_entry *entry;
	int flags;
	int r;

	if (!fs::is_directory(destination)) {
		throw archive_exception("Destination '" + destination + "' is not a directory. Cannot decompress archive.");
	}
	if (!fs::is_regular_file(filename)) {
		throw archive_exception("Source archive '" + filename + "' not exists or is not a regular file.");
	}

	// Select which attributes we want to restore.
	flags = ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_FFLAGS;
	// Don't allow ".." in any path within archive
	flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;

	a = archive_read_new();
	if (a == NULL) {
		throw archive_exception("Cannot create source archive.");
	}
	if (archive_read_support_format_all(a) != ARCHIVE_OK) {
		throw archive_exception("Cannot set formats for source archive.");
	}
	if (archive_read_support_compression_all(a) != ARCHIVE_OK) {
		throw archive_exception("Cannot set compression methods for source archive.");
	}
	ext = archive_write_disk_new();
	if (ext == NULL) {
		throw archive_exception("Cannot allocate archive entry.");
	}
	if (archive_write_disk_set_options(ext, flags) != ARCHIVE_OK) {
		throw archive_exception("Cannot set options for writing to disk.");
	}
	if (archive_write_disk_set_standard_lookup(ext) != ARCHIVE_OK) {
		throw archive_exception("Cannot set lookup for writing to disk.");
	}

	r = archive_read_open_filename(a, filename.c_str(), 10240);
	if (r < ARCHIVE_OK) {
		throw archive_exception("Cannot open source archive.");
	}

	while (true) {
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF) {
			break;
		}
		if (r < ARCHIVE_OK) {
			throw archive_exception(archive_error_string(a));
		}

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

		r = archive_write_header(ext, entry);
		if (r < ARCHIVE_OK) {
			throw archive_exception(archive_error_string(ext));
		}

		if (archive_entry_size(entry) > 0) {
			copy_data(a, ext);
		}

		r = archive_write_finish_entry(ext);
		if (r < ARCHIVE_OK) {
			throw archive_exception(archive_error_string(ext));
		}
	}

	archive_read_close(a);
	archive_read_free(a);
	archive_write_close(ext);
	archive_write_free(ext);
}


void archivator::copy_data(archive *ar, archive *aw)
{
	int r;
	const void *buff;
	size_t size;
	int64_t offset;

	while (true) {
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF) {
			break;
		}
		if (r < ARCHIVE_OK) {
			throw archive_exception(archive_error_string(ar));
		}
		r = archive_write_data_block(aw, buff, size, offset);
		if (r < ARCHIVE_OK) {
			throw archive_exception(archive_error_string(aw));
		}
	}
}

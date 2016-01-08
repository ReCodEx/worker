#include "archivator.h"


void archivator::compress()
{

}


void archivator::decompress(const std::string &filename, const std::string &destination)
{
	archive *a;
	archive *ext;
	archive_entry *entry;
	int flags;
	int r;

	// Select which attributes we want to restore.
	flags = ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_FFLAGS;
	// Don't allow ".." in any path within archive
	flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;

	a = archive_read_new();
	archive_read_support_format_all(a);
	archive_read_support_compression_all(a);
	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);
	archive_write_disk_set_standard_lookup(ext);

	if ((r = archive_read_open_filename(a, filename.c_str(), 10240))) {
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

		const char* current_file = archive_entry_pathname(entry);
		const std::string full_path = (fs::path(destination) / current_file).string();
		archive_entry_set_pathname(entry, full_path.c_str());

		r = archive_write_header(ext, entry);

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
	off_t offset;

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

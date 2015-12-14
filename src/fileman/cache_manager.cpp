#include "cache_manager.h"


cache_manager::cache_manager(std::string caching_dir)
{
	fs::path cache_path(caching_dir);

	try {
		if(!fs::is_directory(cache_path)) {
			fs::create_directories(cache_path);
		}
	} catch (fs::filesystem_error &e) {
		throw fm_exception("Cannot create directory " + caching_dir + e.what());
	}

	caching_dir_ = cache_path;
}

void cache_manager::get_file(std::string src_name, std::string dst_path)
{
	fs::path source_file = caching_dir_ / src_name;
	fs::path destination_file = fs::path(dst_path) / src_name;

	try {
		fs::copy_file(source_file, destination_file, fs::copy_option::overwrite_if_exists);
	} catch (fs::filesystem_error &e) {
		throw fm_exception(std::string("Error copying file: ") + e.what());
	}
}

void cache_manager::put_file(std::string name)
{
	fs::path source_file(name);
	fs::path destination_file = caching_dir_ / source_file.filename();

	try {
		fs::copy_file(source_file, destination_file, fs::copy_option::overwrite_if_exists);
	} catch (fs::filesystem_error &e) {
		throw fm_exception(std::string("Error copying file: ") + e.what());
	}
}


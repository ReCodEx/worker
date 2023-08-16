#include "cache_manager.h"
#include "helpers/string_utils.h"


cache_manager::cache_manager(std::shared_ptr<spdlog::logger> logger)
	: cache_manager(fs::temp_directory_path().string(), logger)
{
}

cache_manager::cache_manager(const std::string &caching_dir, std::shared_ptr<spdlog::logger> logger) : logger_(logger)
{
	if (logger_ == nullptr) { logger_ = helpers::create_null_logger(); }

	fs::path cache_path(caching_dir);

	try {
		if (!fs::is_directory(cache_path)) { fs::create_directories(cache_path); }
	} catch (fs::filesystem_error &e) {
		auto message = "Cannot create directory " + cache_path.string() + ". Error: " + e.what();
		logger_->warn(message);
		throw fm_exception(message);
	}

	caching_dir_ = cache_path;
}

void cache_manager::get_file(const std::string &src_name, const std::string &dst_path)
{
	fs::path source_file = caching_dir_ / fs::path(src_name).relative_path();
	fs::path destination_file = dst_path;
	logger_->debug("Copying file {} from cache to {}", src_name, dst_path);

	if (!fs::is_regular_file(source_file)) {
		auto message = "Cache miss. File " + src_name + " is not present in cache.";
		logger_->debug(message);
		throw fm_exception(message);
	}

	try {
		fs::copy_file(source_file, destination_file, fs::copy_options::overwrite_existing);
		fs::permissions(fs::path(destination_file),
			fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write,
			fs::perm_options::add);
		// change last modification time of the file
		fs::last_write_time(source_file, std::filesystem::file_time_type::min());
	} catch (fs::filesystem_error &e) {
		auto message = "Failed to copy file '" + source_file.string() + "' to '" + dst_path + "'. Error: " + e.what();
		logger_->warn(message);
		throw fm_exception(message);
	}
}

void cache_manager::put_file(const std::string &src_name, const std::string &dst_name)
{
	fs::path source_file(src_name);
	fs::path destination_file = caching_dir_ / fs::path(dst_name).relative_path();
	fs::path destination_temp_file;

	do {
		// generate name and check it for existance, if exists... repeat
		fs::path random_name(dst_name + "-" + helpers::random_alphanum_string(20) + ".tmp");
		destination_temp_file = caching_dir_ / random_name.relative_path();
	} while (fs::exists(destination_temp_file));

	logger_->debug("Copying file {} to cache with name {}", src_name, dst_name);

	try {
		// first copy only temporary file
		fs::copy_file(source_file, destination_temp_file, fs::copy_options::overwrite_existing);
		// and then move (atomically) the file to its original destination
		fs::rename(destination_temp_file, destination_file);
	} catch (fs::filesystem_error &e) {
		auto message = "Failed to copy file " + src_name + " to cache. Error: " + e.what();
		logger_->warn(message);
		throw fm_exception(message);
	}
}

std::string cache_manager::get_caching_dir() const
{
	return caching_dir_.string();
}

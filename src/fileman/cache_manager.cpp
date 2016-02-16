#include "cache_manager.h"


cache_manager::cache_manager(std::shared_ptr<spdlog::logger> logger) :
	cache_manager(fs::temp_directory_path().string(), logger)
{
}

cache_manager::cache_manager(const std::string &caching_dir, std::shared_ptr<spdlog::logger> logger)
{
	if (logger != nullptr) {
		logger_ = logger;
	} else {
		//Create logger manually to avoid global registration of logger
		auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
		logger_ = std::make_shared<spdlog::logger>("cache_manager_nolog", sink);
	}

	fs::path cache_path(caching_dir);

	try {
		if (!fs::is_directory(cache_path)) {
			fs::create_directories(cache_path);
		}
	} catch (fs::filesystem_error &e) {
		auto message = "Cannot create directory " + cache_path.string() + ". Error: " + e.what();
		logger_->warn() << message;
		throw fm_exception(message);
	}

	caching_dir_ = cache_path;
}

void cache_manager::get_file(const std::string &src_name, const std::string &dst_path)
{
	fs::path source_file = caching_dir_ / src_name;
	fs::path destination_file = dst_path;
	logger_->debug() << "Copying file " << src_name + " from cache to " + dst_path;

	if (!fs::is_regular_file(source_file)) {
		auto message = "Cache miss. File " + src_name + " is not present in cache.";
		logger_->debug() << message;
		throw fm_exception(message);
	}

	try {
		fs::copy_file(source_file, destination_file, fs::copy_option::overwrite_if_exists);
	} catch (fs::filesystem_error &e) {
		auto message = "Failed to copy file " + source_file.string() + " to " + dst_path +
				". Error: " + e.what();
		logger_->warn() << message;
		throw fm_exception(message);
	}
}

void cache_manager::put_file(const std::string &src_name, const std::string &dst_name)
{
	fs::path source_file(src_name);
	fs::path destination_file = caching_dir_ / dst_name;
	logger_->debug() << "Copying file " << src_name + " to cache with name " << dst_name;

	try {
		fs::copy_file(source_file, destination_file, fs::copy_option::overwrite_if_exists);
	} catch (fs::filesystem_error &e) {
		auto message = "Failed to copy file " + src_name + " to cache. Error: " + e.what();
		logger_->warn() << message;
		throw fm_exception(message);
	}
}

std::string cache_manager::get_caching_dir() const
{
	return caching_dir_.string();
}

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
		auto sink = std::make_shared<spdlog::sinks::stderr_sink_st>();
		logger_ = std::make_shared<spdlog::logger>("cache_manager_nolog", sink);
		//Set loglevel to 'off' cause no logging
		logger_->set_level(spdlog::level::off);
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
	fs::path destination_file = fs::path(dst_path) / src_name;
	logger_->debug() << "Copying file " << source_file.string() + " from cache to " + dst_path;

	try {
		fs::copy_file(source_file, destination_file, fs::copy_option::overwrite_if_exists);
	} catch (fs::filesystem_error &e) {
		auto message = "Failed to copy file " + source_file.string() + " to " + dst_path +
				". Error: " + e.what();
		logger_->warn() << message;
		throw fm_exception(message);
	}
}

void cache_manager::put_file(const std::string &name)
{
	fs::path source_file(name);
	fs::path destination_file = caching_dir_ / source_file.filename();
	logger_->debug() << "Copying file " << name + " to cache";

	try {
		fs::copy_file(source_file, destination_file, fs::copy_option::overwrite_if_exists);
	} catch (fs::filesystem_error &e) {
		auto message = "Failed to copy file " + name + " to cache. Error: " + e.what();
		logger_->warn() << message;
		throw fm_exception(message);
	}
}

void cache_manager::set_params(const std::string &destination, const std::string &, const std::string &)
{
	fs::path cache_path(destination);

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

std::string cache_manager::get_destination() const
{
	return caching_dir_.string();
}

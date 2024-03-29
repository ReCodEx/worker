#ifndef RECODEX_WORKER_CACHE_MANAGER_H
#define RECODEX_WORKER_CACHE_MANAGER_H

#include <string>
#include <memory>
#include <filesystem>
#include "file_manager_interface.h"
#include "helpers/logger.h"

namespace fs = std::filesystem;


/**
 * Manager of local files cache.
 *
 * Cache is a directory inside host filesystem, where recently used files
 * are stored for some period of time. This directory could be the same for
 * more worker instances. Removing old files will do recodex-cleaner project.
 * Failed operations throws @a fm_exception exception.
 */
class cache_manager : public file_manager_interface
{
public:
	/**
	 * Constructor with optional logger.
	 * @param logger Shared pointer to system logger (optional).
	 */
	cache_manager(std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Set up cache manager with working directory.
	 * @param caching_dir Directory where cached files will be stored. If this directory don't exist, it'll be created.
	 * @param logger Shared pointer to system logger (optional).
	 */
	cache_manager(const std::string &caching_dir, std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Destructor.
	 */
	~cache_manager() override = default;
	/**
	 * Copy a file from cache to destination.
	 * @param src_name Name of the file without path.
	 * @param dst_name Name of the destination path with requested filename - the file
	 *					can be renamed during fetching.
	 */
	void get_file(const std::string &src_name, const std::string &dst_name) override;
	/**
	 * Copy file to cache.
	 * @param src_name Path and name of the file to be copied.
	 * @param dst_name Name of the file in cache.
	 */
	void put_file(const std::string &src_name, const std::string &dst_name) override;

	/**
	 * Get path to the directory where files are stored.
	 */
	std::string get_caching_dir() const;

private:
	/** Path to the caching directory. */
	fs::path caching_dir_;
	/** System or null logger. */
	std::shared_ptr<spdlog::logger> logger_;
};

#endif // RECODEX_WORKER_CACHE_MANAGER_H

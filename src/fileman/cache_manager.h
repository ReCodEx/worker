#ifndef RECODEX_WORKER_CACHE_MANAGER_H
#define RECODEX_WORKER_CACHE_MANAGER_H

#include <string>
#include <memory>
#include "file_manager_base.h"
#include "../helpers/logger.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


/**
 * Manager of local file cache.
 * Failed operations throws @a fm_exception exception.
 */
class cache_manager : public file_manager_base
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
	virtual ~cache_manager()
	{
	}
	/**
	 * Copy a file from cache to destination.
	 * @param src_name Name of the file without path.
	 * @param dst_name Name of the destination path with requested filename.
	 */
	virtual void get_file(const std::string &src_name, const std::string &dst_name);
	/**
	 * Copy file to cache.
	 * @param src_name Path and name of the file to be copied.
	 * @param dst_name Name of the file in cache.
	 */
	virtual void put_file(const std::string &src_name, const std::string &dst_name);

	/**
	 * Get the directory where files are stored
	 */
	std::string get_caching_dir() const;

private:
	fs::path caching_dir_;
	std::shared_ptr<spdlog::logger> logger_;
};

#endif // RECODEX_WORKER_CACHE_MANAGER_H

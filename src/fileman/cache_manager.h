
#ifndef CODEX_WORKER_CACHE_MANAGER_H
#define CODEX_WORKER_CACHE_MANAGER_H

#include <string>
#include "file_manager_base.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


class cache_manager : public file_manager_base {
public:
	/**
	 * Set up cache manager with working directory.
	 *
	 * @param caching_dir Directory where cached files will be stored. If this directory don't exist, it'll be created.
	 */
    cache_manager(std::string caching_dir);
    virtual ~cache_manager() {}
	/**
	 * Copy a file from cache to destination.
	 *
	 * @param src_name Name of the file without path.
	 * @param dst_path Name of the destination path without filename.
	 */
    virtual void get_file(std::string src_name, std::string dst_path);
	/**
	 * Copy file from path to cache.
	 *
	 * @param name Path and name of the file to be copied.
	 */
    virtual void put_file(std::string name);
private:
    fs::path caching_dir_;
};

#endif //CODEX_WORKER_CACHE_MANAGER_H

#ifndef CODEX_WORKER_FILE_MANAGER_H
#define CODEX_WORKER_FILE_MANAGER_H

#include "file_manager_base.h"
#include "cache_manager.h"
#include "http_manager.h"
#include <utility>


/**
 * File manager class.
 * This class combines @a http_manager and @a cache_manager classes together.
 * Failed operations throws @a fm_exception exception.
 */
class file_manager : public file_manager_base {
public:
	typedef std::unique_ptr<cache_manager> cache_man_ptr;
	typedef std::unique_ptr<http_manager> http_man_ptr;
public:
	/**
	 * Constructor for creating and initializing @a cache_manager and @a http_manager.
	 * @param caching_dir Local directory, where cached files are stored.
	 * @param remote_url URL address of remote server
	 * @param username Username for HTTP Basic Authentication
	 * @param password Password for HTTP Basic Authentication
	 * @param logger Shared pointer to system logger (optional).
	 */
	file_manager(const std::string &caching_dir, const std::string &remote_url, const std::string &username,
				 const std::string &password, std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Constructor for creating this class with instances of @a cache_manager and @a http_manager.
	 * This is useful especially for testing with mocked classes.
	 * @param cache_manager Cache manager poiter.
	 * @param http_manager HTTP manager pointer.
	 */
	file_manager(cache_man_ptr cache_manager, http_man_ptr http_manager);
	/**
	 * Destructor.
	 */
    virtual ~file_manager() {}
	/**
	 * Get file. If requested file is in cache, it'll be copied to @a dst_path immediately,
	 * otherwise it'll be downloaded to cache first.
	 * @param src_name Name of requested file.
	 * @param dst_path Path where to save the file.
	 */
	virtual void get_file(const std::string &src_name, const std::string &dst_path);
	/**
	 * Upload file to remote server. It won't be saved to cache.
	 * @param name Name of the file to upload.
	 */
	virtual void put_file(const std::string &name);
	/**
	 * Alter parameters at runtime.
	 * @param destination URL of remote server
	 * @param username Username for HTTP Basic Authentication
	 * @param password Password for HTTP Basic Authentication
	 */
	virtual void set_params(const std::string &destination, const std::string &username,
						  const std::string &password);
	/**
	 * Return actual URL of remote server.
	 */
	virtual std::string get_destination() const;
private:
	cache_man_ptr cache_man_;
	http_man_ptr http_man_;
};

#endif //CODEX_WORKER_FILE_MANAGER_H

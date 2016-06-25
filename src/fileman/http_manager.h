#ifndef RECODEX_WORKER_HTTP_MANAGER_H
#define RECODEX_WORKER_HTTP_MANAGER_H

#include <string>
#include <memory>
#include "file_manager_base.h"
#include "../helpers/logger.h"
#include "../config/fileman_config.h"


/**
 * Class for managing transfer over HTTP connection.
 * Failed operations throws @ref fm_exception exception.
 */
class http_manager : public file_manager_base
{
public:
	/**
	  * Default constructor, optionally can set a system logger.
	  * @param logger Shared pointer to system logger (optional).
	  */
	http_manager(std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Constructor with initialization.
	 * @param configs File server configurations
	 * @param logger Shared pointer to system logger (optional).
	 */
	http_manager(const std::vector<fileman_config> &configs, std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Destructor.
	 */
	virtual ~http_manager()
	{
	}
	/**
	 * Get and save file locally.
	 * @param src_name Name of requested file (without path)
	 * @param dst_name Path to the directory with name of the created file.
	 */
	virtual void get_file(const std::string &src_name, const std::string &dst_name);
	/**
	 * Upload file to remote server with HTTP PUT method.
	 * @param src_name Name with path to a file to upload.
	 * @param dst_url Url where the file will be uploaded.
	 */
	virtual void put_file(const std::string &src_name, const std::string &dst_url);

protected:
	/**
	 * Finds the configuration for a file server that matches given URL
	 * The pointer returned by this method is valid as long as this object exists
	 * @param url The URL used to determine the file server
	 * @return A pointer to the configuration, or a nullptr when nothing is found
	 */
	const fileman_config *find_config(const std::string &url) const;

private:
	const std::vector<fileman_config> configs_;
	std::shared_ptr<spdlog::logger> logger_;
};

#endif // RECODEX_WORKER_HTTP_MANAGER_H

#ifndef CODEX_WORKER_HTTP_MANAGER_H
#define CODEX_WORKER_HTTP_MANAGER_H

#include <string>
#include <memory>
#include "file_manager_base.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "../config/fileman_config.h"


/**
 * Class for managing transfer over HTTP connection.
 * Failed operations throws @a fm_exception exception.
 */
class http_manager : public file_manager_base {
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
	http_manager(const std::vector<fileman_config> &configs,
				 std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Destructor.
	 */
	virtual ~http_manager() {}
	/**
	 * Get and save file locally.
	 * @param src_name Name of requested file (without path)
	 * @param dst_path Path to directory, where the file will be saved.
	 */
	virtual void get_file(const std::string &src_name, const std::string &dst_path);
	/**
	 * Upload file to remote server with HTTP PUT method.
	 * @param name Name with path to a file to upload.
	 */
	virtual void put_file(const std::string &src_name, const std::string &dst_path);

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

#endif //CODEX_WORKER_HTTP_MANAGER_H

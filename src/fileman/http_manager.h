#ifndef CODEX_WORKER_HTTP_MANAGER_H
#define CODEX_WORKER_HTTP_MANAGER_H

#include <string>
#include <memory>
#include "file_manager_base.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"



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
	 * @param username Username for HTTP Basic Authentication
	 * @param password Password for HTTP Basic Authentication
	 * @param logger Shared pointer to system logger (optional).
	 */
	http_manager(const std::string &username, const std::string &password,
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
	/**
	 * Change parameters in runtime.
	 * @param username Username for HTTP Basic Authentication
	 * @param password Password for HTTP Basic Authentication
	 */
	virtual void set_params(const std::string &username, const std::string &password);

private:
	std::string username_;
	std::string password_;
	std::shared_ptr<spdlog::logger> logger_;
};

#endif //CODEX_WORKER_HTTP_MANAGER_H

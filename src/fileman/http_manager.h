#ifndef CODEX_WORKER_HTTP_MANAGER_H
#define CODEX_WORKER_HTTP_MANAGER_H

// Include of WinSock2 has to be before including curl.h
#ifdef _WIN32
#include <WinSock2.h>
#endif

#include <string>
#include <memory>
#include "file_manager_base.h"
#include "spdlog/spdlog.h"



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
	 * @param remote_url URL address of remote server
	 * @param username Username for HTTP Basic Authentication
	 * @param password Password for HTTP Basic Authentication
	 * @param logger Shared pointer to system logger (optional).
	 */
	http_manager(const std::string &remote_url, const std::string &username, const std::string &password,
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
	virtual void put_file(const std::string &name);
	/**
	 * Change parameters in runtime.
	 * @param destination URL address of remote server
	 * @param username Username for HTTP Basic Authentication
	 * @param password Password for HTTP Basic Authentication
	 */
	virtual void set_params(const std::string &destination, const std::string &username, const std::string &password);
	/**
	 * Get actual remote URL.
	 * @return destination
	 */
	virtual std::string get_destination() const;
private:
	std::string remote_url_;
    std::string username_;
    std::string password_;
	std::shared_ptr<spdlog::logger> logger_;
	void validate_url();
};

#endif //CODEX_WORKER_HTTP_MANAGER_H

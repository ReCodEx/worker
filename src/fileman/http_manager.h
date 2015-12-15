
#ifndef CODEX_WORKER_HTTP_MANAGER_H
#define CODEX_WORKER_HTTP_MANAGER_H

#include <string>
#include "file_manager_base.h"




class http_manager : public file_manager_base {
public:
	http_manager() = default;
	http_manager(const std::string &remote_url, const std::string &username, const std::string &password);
    virtual ~http_manager() {}
	virtual void get_file(const std::string &src_name, const std::string &dst_path);
	virtual void put_file(const std::string &name);
	virtual void set_data(const std::string &destination, const std::string &username, const std::string &password);
	/**
	 * Get already set remote url.
	 * @return destination
	 */
	virtual std::string get_destination() const;
private:
	std::string remote_url_;
    std::string username_;
    std::string password_;
	void validate_url();
};

#endif //CODEX_WORKER_HTTP_MANAGER_H

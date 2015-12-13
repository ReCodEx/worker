
#ifndef CODEX_WORKER_HTTP_MANAGER_H
#define CODEX_WORKER_HTTP_MANAGER_H

#include <string>
#include "file_manager_base.h"




class http_manager : public file_manager_base {
public:
	http_manager() = default;
    http_manager(std::string remote_url, std::string username, std::string password);
    virtual ~http_manager() {}
    virtual void get_file(std::string src_name, std::string dst_path);
    virtual void put_file(std::string name);
	void set_data(std::string remote_url, std::string username, std::string password);
private:
	std::string remote_url_;
    std::string username_;
    std::string password_;
};

#endif //CODEX_WORKER_HTTP_MANAGER_H

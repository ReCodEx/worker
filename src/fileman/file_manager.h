
#ifndef CODEX_WORKER_FILE_MANAGER_H
#define CODEX_WORKER_FILE_MANAGER_H

#include "file_manager_base.h"
#include "cache_manager.h"
#include "http_manager.h"
#include <utility>


class file_manager : public file_manager_base {
public:
	typedef std::unique_ptr<cache_manager> cache_man_ptr;
	typedef std::unique_ptr<http_manager> http_man_ptr;
public:
	file_manager(const std::string &caching_dir, const std::string &remote_url,
				 const std::string &username, const std::string &password);
	file_manager(cache_man_ptr cache_manager, http_man_ptr http_manager);
    virtual ~file_manager() {}
	virtual void get_file(const std::string &src_name, const std::string &dst_path);
	virtual void put_file(const std::string &name);
	virtual void set_data(const std::string &destination, const std::string &username,
						  const std::string &password);
	virtual std::string get_destination() const;
private:
	cache_man_ptr cache_man_;
	http_man_ptr http_man_;
};

#endif //CODEX_WORKER_FILE_MANAGER_H

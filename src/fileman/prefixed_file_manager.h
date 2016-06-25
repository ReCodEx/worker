#ifndef RECODEX_WORKER_PREFIXED_FILE_MANAGER_H
#define RECODEX_WORKER_PREFIXED_FILE_MANAGER_H

#include <memory>
#include "file_manager_base.h"

class prefixed_file_manager : public file_manager_base
{
private:
	const std::string prefix_;
	std::shared_ptr<file_manager_base> fm_;

public:
	prefixed_file_manager(std::shared_ptr<file_manager_base> fm, const std::string &prefix);
	virtual void get_file(const std::string &src_name, const std::string &dst_name);
	virtual void put_file(const std::string &src_name, const std::string &dst_name);
};


#endif // RECODEX_WORKER_PREFIXED_FILE_MANAGER_H

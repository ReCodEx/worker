#ifndef CODEX_WORKER_JOB_CALLBACK_H
#define CODEX_WORKER_JOB_CALLBACK_H

#include <memory>
#include <string>

#include "fileman/file_manager_base.h"

struct job_callback {
public:
	job_callback (std::shared_ptr<file_manager_base>);
	void operator() (const std::string &, const std::string &, const std::string &);
private:
	std::shared_ptr<file_manager_base> fm_;
};

#endif //CODEX_WORKER_JOB_CALLBACK_H

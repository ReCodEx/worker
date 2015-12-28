#ifndef CODEX_WORKER_JOB_HPP
#define CODEX_WORKER_JOB_HPP

#include <vector>
#include "task_base.h"

/**
 *
 */
class job {
public:
	void run();
private:
	std::vector<task_base> tasks_;
};

#endif //CODEX_WORKER_JOB_HPP

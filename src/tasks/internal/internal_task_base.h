#ifndef CODEX_WORKER_INTERNAL_TASK_BASE_HPP
#define CODEX_WORKER_INTERNAL_TASK_BASE_HPP

#include "../task_base.h"


/**
 *
 */
class internal_task_base : task_base {
public:
	virtual void run() = 0;
};

#endif //CODEX_WORKER_INTERNAL_TASK_BASE_HPP

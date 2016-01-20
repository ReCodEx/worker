#ifndef CODEX_WORKER_INTERNAL_MKDIR_TASK_H
#define CODEX_WORKER_INTERNAL_MKDIR_TASK_H

#include "../task_base.h"


/**
 *
 */
class mkdir_task : public task_base {
public:
	mkdir_task(size_t id, std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::vector<std::string> &dependencies);
	virtual ~mkdir_task();
	virtual std::shared_ptr<task_results> run();
};

#endif //CODEX_WORKER_INTERNAL_MKDIR_TASK_H

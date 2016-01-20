#ifndef CODEX_WORKER_INTERNAL_EXTRACT_TASK_H
#define CODEX_WORKER_INTERNAL_EXTRACT_TASK_H

#include "../task_base.h"


/**
 * Extract archive. Requires 2 arguments - archive name and directory to extract to.
 */
class extract_task : public task_base {
public:
	extract_task(size_t id, std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::vector<std::string> &dependencies);
	virtual ~extract_task();
	virtual std::shared_ptr<task_results> run();
};

#endif //CODEX_WORKER_INTERNAL_EXTRACT_TASK_H

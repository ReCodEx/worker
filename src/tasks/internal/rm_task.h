#ifndef CODEX_WORKER_INTERNAL_RM_TASK_H
#define CODEX_WORKER_INTERNAL_RM_TASK_H

#include "../task_base.h"


/**
 * Remove all files and directories as specified. Calls boost::filesystem::remove_all().
 */
class rm_task : public task_base {
public:
	rm_task(size_t id, std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::string &log,
			const std::vector<std::string> &dependencies);
	virtual ~rm_task();
	virtual void run();
};

#endif //CODEX_WORKER_INTERNAL_RM_TASK_H

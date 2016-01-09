#ifndef CODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H
#define CODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H

#include "../task_base.h"


/**
 * Create archive. Requires 2 arguments - directory to be archived and name of the archive.
 */
class archivate_task : public task_base {
public:
	archivate_task(size_t id, std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::string &log,
			const std::vector<std::string> &dependencies);
	virtual ~archivate_task();
	virtual void run();
};

#endif //CODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H

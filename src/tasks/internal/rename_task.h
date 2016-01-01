#ifndef CODEX_WORKER_INTERNAL_RENAME_TASK_H
#define CODEX_WORKER_INTERNAL_RENAME_TASK_H

#include "../task_base.h"


/**
 * Rename files. Calls boost::filesystem::rename. For detailed behavior listing see
 * POSIX rename() function - http://pubs.opengroup.org/onlinepubs/000095399/functions/rename.html
 */
class rename_task : task_base {
public:
	rename_task(std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::string &log,
			const std::vector<std::string> &dependencies);
	virtual ~rename_task();
	virtual void run();
};

#endif //CODEX_WORKER_INTERNAL_RENAME_TASK_H

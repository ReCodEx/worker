#ifndef CODEX_WORKER_INTERNAL_FETCH_TASK_H
#define CODEX_WORKER_INTERNAL_FETCH_TASK_H

#include "../../fileman/file_manager_base.h"
#include "../task_base.h"
#include <memory>


/**
 * Fetch files from remote server. Requires 2 argument: filename to get/download and destination directory.
 */
class fetch_task : public task_base
{
public:
	fetch_task(size_t id, std::shared_ptr<task_metadata> task_meta, std::shared_ptr<file_manager_base> filemanager);
	virtual ~fetch_task();
	virtual std::shared_ptr<task_results> run();

private:
	std::shared_ptr<file_manager_base> filemanager_;
};

#endif // CODEX_WORKER_INTERNAL_FETCH_TASK_H

#ifndef CODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H
#define CODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H

#include "../task_base.h"


/**
 * Create archive. Requires 2 arguments - directory to be archived and name of the archive.
 */
class archivate_task : public task_base
{
public:
	archivate_task(size_t id, task_metadata task_meta);
	virtual ~archivate_task();
	virtual std::shared_ptr<task_results> run();
};

#endif // CODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H

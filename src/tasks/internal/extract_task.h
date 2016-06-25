#ifndef RECODEX_WORKER_INTERNAL_EXTRACT_TASK_H
#define RECODEX_WORKER_INTERNAL_EXTRACT_TASK_H

#include "../task_base.h"


/**
 * Extract archive. Requires 2 arguments - archive name and directory to extract to.
 */
class extract_task : public task_base
{
public:
	extract_task(size_t id, std::shared_ptr<task_metadata> task_meta);
	virtual ~extract_task();
	virtual std::shared_ptr<task_results> run();
};

#endif // RECODEX_WORKER_INTERNAL_EXTRACT_TASK_H

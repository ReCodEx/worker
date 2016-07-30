#ifndef RECODEX_WORKER_TASK_FACTORY_BASE_H
#define RECODEX_WORKER_TASK_FACTORY_BASE_H

#include <memory>
#include "task_base.h"
#include "create_params.h"
#include "../fileman/file_manager_interface.h"


/**
 * Interface for task factories.
 */
class task_factory_interface
{
public:
	/**
	 * Virtual destructor for proper destruction of inherited classes.
	 */
	virtual ~task_factory_interface()
	{
	}

	/**
	 * Create internal task. This could be one of predefined operations like move or copy file,
	 * extract archive etc. which don't need sandbox to execute or root task, ie. base holder
	 * for all tasks in dependency graph.
	 * @param id Identifier of newly created task.
	 * @param task_meta Metadata for newly created task.
	 * @return Pointer to task's base type holding proper task type.
	 */
	virtual std::shared_ptr<task_base> create_internal_task(
		size_t id, std::shared_ptr<task_metadata> task_meta = nullptr) = 0;

	/**
	 * Created task which will run in sandboxed environment.
	 * @param data Structure holding creating parameters for external (sandboxed) tasks.
	 * @return Pointer to task's base type holding proper task type.
	 */
	virtual std::shared_ptr<task_base> create_sandboxed_task(const create_params &data) = 0;
};


#endif // RECODEX_WORKER_TASK_FACTORY_BASE_H

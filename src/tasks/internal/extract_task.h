#ifndef RECODEX_WORKER_INTERNAL_EXTRACT_TASK_H
#define RECODEX_WORKER_INTERNAL_EXTRACT_TASK_H

#include "tasks/task_base.h"


/**
 * Extract archive using @ref archivator. Requires 2 arguments - archive name and directory to extract to.
 */
class extract_task : public task_base
{
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identificator of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has just 2 arguments - archive name and directory to extract to.
	 * For more info about archivation see @ref archivator class.
	 * @throws task_exception on invalid number of arguments.
	 */
	extract_task(size_t id, std::shared_ptr<task_metadata> task_meta);
	/**
	 * Destructor.
	 */
	~extract_task() override = default;
	/**
	 * Run the action.
	 * @return Evaluation results to be pushed back to frontend.
	 */
	std::shared_ptr<task_results> run() override;
};

#endif // RECODEX_WORKER_INTERNAL_EXTRACT_TASK_H

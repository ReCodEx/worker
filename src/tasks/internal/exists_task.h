#ifndef RECODEX_WORKER_INTERNAL_EXISTS_TASK_H
#define RECODEX_WORKER_INTERNAL_EXISTS_TASK_H

#include "tasks/task_base.h"


/**
 * Check if file or folder exists.
 */
class exists_task : public task_base
{
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identificator of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has at least one argument - names of files/folders which should be checked.
	 * @throws task_exception when wrong arguments provided.
	 */
	exists_task(size_t id, std::shared_ptr<task_metadata> task_meta);
	/**
	 * Destructor.
	 */
	~exists_task() override = default;
	/**
	 * Run the action.
	 * @return Evaluation results to be pushed back to frontend.
	 */
	std::shared_ptr<task_results> run() override;
};

#endif // RECODEX_WORKER_INTERNAL_EXISTS_TASK_H

#ifndef RECODEX_WORKER_INTERNAL_CP_TASK_H
#define RECODEX_WORKER_INTERNAL_CP_TASK_H

#include "tasks/task_base.h"


/**
 * Copy files using std::filesystem::copy.
 */
class cp_task : public task_base
{
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identifier of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has just 2 arguments -
	 * http://www.boost.org/doc/libs/1_59_0_b1/libs/filesystem/doc/reference.html#copy.
	 * @throws task_exception on invalid number of arguments.
	 */
	cp_task(std::size_t id, std::shared_ptr<task_metadata> task_meta);
	/**
	 * Destructor.
	 */
	~cp_task() override = default;
	/**
	 * Run the action.
	 * @return Evaluation results to be pushed back to frontend.
	 */
	std::shared_ptr<task_results> run() override;
};

#endif // RECODEX_WORKER_INTERNAL_CP_TASK_H

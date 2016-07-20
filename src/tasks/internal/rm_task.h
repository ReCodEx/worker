#ifndef RECODEX_WORKER_INTERNAL_RM_TASK_H
#define RECODEX_WORKER_INTERNAL_RM_TASK_H

#include "../task_base.h"


/**
 * Remove all files and directories as specified.
 */
class rm_task : public task_base
{
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identificator of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has at least one argument - names of files and directories to be removed.
	 * @throws task_exception when no argument provided.
	 */
	rm_task(size_t id, std::shared_ptr<task_metadata> task_meta);
	/**
	 * Destructor.
	 */
	virtual ~rm_task();
	/**
	 * Run the action. It tries to delete all entries first. When any of items cannot be deleted,
	 * exception is throuwn, otherwise normal result is returned. For more info about removing function see
	 * http://www.boost.org/doc/libs/1_59_0_b1/libs/filesystem/doc/reference.html#remove_all.
	 * @return Evaluation results to be pushed back to frontend.
	 * @throws task_exception if any of entries cannot be deleted.
	 */
	virtual std::shared_ptr<task_results> run();
};

#endif // RECODEX_WORKER_INTERNAL_RM_TASK_H

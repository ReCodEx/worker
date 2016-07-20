#ifndef RECODEX_WORKER_INTERNAL_MKDIR_TASK_H
#define RECODEX_WORKER_INTERNAL_MKDIR_TASK_H

#include "../task_base.h"


/**
 * Make one or more directories.
 */
class mkdir_task : public task_base
{
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identificator of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has at least one argument - names of directories to be created.
	 * @throws task_exception when no argument provided.
	 */
	mkdir_task(size_t id, std::shared_ptr<task_metadata> task_meta);
	/**
	 * Destructor.
	 */
	virtual ~mkdir_task();
	/**
	 * Run the action. For every created directory the group write and others write permissions
	 * are added to default ones. For more info about directory creation see
	 * http://www.boost.org/doc/libs/1_59_0_b1/libs/filesystem/doc/reference.html#create_directories.
	 * @return Evaluation results to be pushed back to frontend.
	 * @throws task_exception if any of directories cannot be created. When this happens, all already
	 * created directories are removed.
	 */
	virtual std::shared_ptr<task_results> run();
};

#endif // RECODEX_WORKER_INTERNAL_MKDIR_TASK_H

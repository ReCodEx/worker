#ifndef RECODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H
#define RECODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H

#include "tasks/task_base.h"


/**
 * Create archive using @ref archivator.
 */
class archivate_task : public task_base
{
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identificator of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has just 2 arguments - directory to be archived and name of the archive.
	 * For more info about archivation see @ref archivator class.
	 * @throws task_exception on invalid number of arguments.
	 */
	archivate_task(size_t id, std::shared_ptr<task_metadata> task_meta);
	/**
	 * Destructor.
	 */
	~archivate_task() override = default;
	/**
	 * Run the action.
	 * @return Evaluation results to be pushed back to frontend.
	 */
	std::shared_ptr<task_results> run() override;
};

#endif // RECODEX_WORKER_INTERNAL_ARCHIVATE_TASK_H

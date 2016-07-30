#ifndef RECODEX_WORKER_INTERNAL_FETCH_TASK_H
#define RECODEX_WORKER_INTERNAL_FETCH_TASK_H

#include "../task_base.h"
#include "../../fileman/file_manager_interface.h"
#include <memory>


/**
 * Fetch files from remote server.
 */
class fetch_task : public task_base
{
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identificator of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has just 2 arguments - filename to get/download and destination directory.
	 * @param filemanager Filemanager which needs to gather requested file.
	 * @throws task_exception on invalid number of arguments.
	 */
	fetch_task(size_t id, std::shared_ptr<task_metadata> task_meta, std::shared_ptr<file_manager_interface> filemanager);
	/**
	 * Destructor.
	 */
	virtual ~fetch_task();
	/**
	 * Run the action.
	 * @return Evaluation results to be pushed back to frontend.
	 * @throws task_exception on fetching error.
	 */
	virtual std::shared_ptr<task_results> run();

private:
	/** Pointer to filemanager instance. */
	std::shared_ptr<file_manager_interface> filemanager_;
};

#endif // RECODEX_WORKER_INTERNAL_FETCH_TASK_H

#ifndef RECODEX_WORKER_TRUNCATE_TASK_H
#define RECODEX_WORKER_TRUNCATE_TASK_H

#include "../task_base.h"


class truncate_task: public task_base {
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identifier of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has just 2 arguments - the name of the file to be truncated and the desired size
	 * @throws task_exception on invalid number of arguments.
	 */
	truncate_task(size_t id, std::shared_ptr<task_metadata> task_meta);

	virtual ~truncate_task();

	virtual std::shared_ptr<task_results> run();

};


#endif //RECODEX_WORKER_TRUNCATE_TASK_H

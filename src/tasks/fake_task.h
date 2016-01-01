#ifndef CODEX_WORKER_FAKE_TASK_HPP
#define CODEX_WORKER_FAKE_TASK_HPP

#include <memory>
#include "task_base.h"


/**
 * Task which does not have any specific meaning and its function is more or less logical.
 * Can be used for root task or as logical parent for some group of tasks.
 * Run method have no function and is empty.
 */
class fake_task : public task_base {
public:
	/**
	 * Only send some fake information to task_base.
	 */
	fake_task();
	/**
	 * Empty destructor.
	 */
	virtual ~fake_task();

	/**
	 * Empty function. Has to be stated for completion.
	 */
	virtual void run();
};

#endif //CODEX_WORKER_FAKE_TASK_HPP

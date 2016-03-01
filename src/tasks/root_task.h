#ifndef CODEX_WORKER_FAKE_TASK_HPP
#define CODEX_WORKER_FAKE_TASK_HPP

#include <memory>
#include "task_base.h"


/**
 * Task which does not have any specific meaning and its function is more or less logical.
 * Can be used for root task or as logical parent for some group of tasks.
 * Run method have no function and is empty.
 */
class root_task : public task_base
{
public:
	root_task() = delete;
	/**
	 * Only send some fake information to task_base.
	 */
	root_task(size_t id, std::shared_ptr<task_metadata> task_meta = std::make_shared<task_metadata>());
	/**
	 * Empty destructor.
	 */
	virtual ~root_task();

	/**
	 * Empty function. Has to be stated for completion.
	 */
	virtual std::shared_ptr<task_results> run();
};

#endif // CODEX_WORKER_FAKE_TASK_HPP

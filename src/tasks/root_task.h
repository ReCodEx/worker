#ifndef RECODEX_WORKER_FAKE_TASK_HPP
#define RECODEX_WORKER_FAKE_TASK_HPP

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
	/**
	 * Default constructor is disabled.
	 */
	root_task() = delete;
	/**
	 * Only send some fake information to task_base.
	 * @param id Unique identificator of load order of tasks.
	 * @param task_meta Variable containing further info about task.
	 */
	root_task(std::size_t id, std::shared_ptr<task_metadata> task_meta = std::make_shared<task_metadata>());
	/**
	 * Empty destructor.
	 */
	~root_task() override = default;

	/**
	 * Empty function. Has to be stated for completeness.
	 * @return Always @a nullptr.
	 */
	std::shared_ptr<task_results> run() override;
};

#endif // RECODEX_WORKER_FAKE_TASK_HPP

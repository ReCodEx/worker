#include "root_task.h"

root_task::root_task(std::size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
}

std::shared_ptr<task_results> root_task::run()
{
	// Nothing to do here... if evaluation will be recursive,
	// then there will be calling of run() methods on children.
	return nullptr;
}

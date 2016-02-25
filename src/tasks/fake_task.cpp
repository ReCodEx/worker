#include "fake_task.h"

fake_task::fake_task(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
}

fake_task::~fake_task()
{
}

std::shared_ptr<task_results> fake_task::run()
{
	// Nothing to do here... if evaluation will be recursive,
	// then there will be calling of run() methods on children.
	return nullptr;
}

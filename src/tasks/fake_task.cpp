#include "fake_task.h"

fake_task::fake_task(size_t id, std::string task_id, size_t priority, bool fatal,
					 std::vector<std::string> deps,
					 std::string cmd, std::vector<std::string> args,
					 std::string log)
	: task_base(id, task_id, priority, fatal, deps, cmd, args, log)
{}

fake_task::~fake_task()
{}

void fake_task::run()
{
	// Nothing to do here... if evaluation will be recursive,
	// then there will be calling of run() methods on children.
	return;
}

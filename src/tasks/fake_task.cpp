#include "fake_task.h"

fake_task::fake_task()
	: task_base("", 0, false, "", "", std::vector<std::string>())
{}

void fake_task::run()
{
	// Nothing to do here, if evaluation will be recursive,
	// then there will be calling of run() methods on children.
	return;
}

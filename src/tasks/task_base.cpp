#include "task_base.h"

task_base::task_base(std::string task_id, size_t priority, bool fatal,
					 std::string cmd, std::string log, std::vector<std::string> dependencies)
	: task_id_(task_id), priority_(priority), fatal_failure_(fatal),
	  cmd_(cmd), log_(log), dependencies_(dependencies)
{}

task_base::~task_base()
{}

void task_base::add_children(std::shared_ptr<task_base> add)
{
	children_.push_back(add);
	return;
}

void task_base::add_parent(std::shared_ptr<task_base> add)
{
	parents_.push_back(add);
	return;
}

const std::string &task_base::get_task_id()
{
	return task_id_;
}

size_t task_base::get_priority()
{
	return priority_;
}

bool task_base::get_fatal_failure()
{
	return fatal_failure_;
}

const std::string &task_base::get_cmd()
{
	return cmd_;
}

const std::string &task_base::get_log()
{
	return log_;
}

const std::vector<std::string> &task_base::get_dependencies()
{
	return dependencies_;
}

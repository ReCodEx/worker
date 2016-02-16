#include "task_base.h"

task_base::task_base(size_t id,
	std::string task_id,
	size_t priority,
	bool fatal,
	const std::vector<std::string> &dependencies,
	const std::string &cmd,
	const std::vector<std::string> &arguments)
	: id_(id), task_id_(task_id), priority_(priority), fatal_failure_(fatal), cmd_(cmd), dependencies_(dependencies),
	  arguments_(arguments), execute_(true)
{
}

task_base::~task_base()
{
}

void task_base::add_children(std::shared_ptr<task_base> add)
{
	if (add == nullptr) {
		return;
	}

	children_.push_back(add);
	return;
}

const std::vector<std::shared_ptr<task_base>> &task_base::get_children()
{
	return children_;
}

void task_base::add_parent(std::shared_ptr<task_base> add)
{
	if (add == nullptr) {
		return;
	}

	parents_.push_back(add);
	return;
}

size_t task_base::get_id()
{
	return id_;
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

const std::vector<std::string> &task_base::get_args()
{
	return arguments_;
}

const std::vector<std::string> &task_base::get_dependencies()
{
	return dependencies_;
}

bool task_base::is_executable()
{
	return execute_;
}

void task_base::set_execution(bool set)
{
	execute_ = set;
	return;
}

void task_base::set_children_execution(bool set)
{
	for (auto &i : children_) {
		i->set_execution(set);
	}
}

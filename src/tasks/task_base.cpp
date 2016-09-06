#include "task_base.h"

task_base::task_base(size_t id, std::shared_ptr<task_metadata> task_meta)
	: id_(id), task_meta_(task_meta), execute_(true)
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
	return task_meta_->task_id;
}

size_t task_base::get_priority()
{
	return task_meta_->priority;
}

bool task_base::get_fatal_failure()
{
	return task_meta_->fatal_failure;
}

const std::string &task_base::get_cmd()
{
	return task_meta_->binary;
}

const std::vector<std::string> &task_base::get_args()
{
	return task_meta_->cmd_args;
}

const std::vector<std::string> &task_base::get_dependencies()
{
	return task_meta_->dependencies;
}

task_type task_base::get_type()
{
	return task_meta_->type;
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

#include "fetch_task.h"


fetch_task::fetch_task(size_t id, std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::vector<std::string> &dependencies,
					   std::shared_ptr<file_manager_base> filemanager)
	: task_base(id, task_id, priority, fatal, dependencies, cmd, arguments), filemanager_(filemanager)
{
	if (arguments_.size() != 2) {
		throw task_exception("Wrong number of arguments. Required: 2, Actual: " + arguments_.size());
	}
}


fetch_task::~fetch_task()
{
}


void fetch_task::run()
{
	try {
		filemanager_->get_file(arguments_[0], arguments_[1]);
	} catch (fm_exception &e) {
		throw task_exception(std::string("Cannot fetch files. Error: ") + e.what());
	}
}

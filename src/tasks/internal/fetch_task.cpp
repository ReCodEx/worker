#include "fetch_task.h"


fetch_task::fetch_task(
	size_t id, std::shared_ptr<task_metadata> task_meta, std::shared_ptr<file_manager_interface> filemanager)
	: task_base(id, task_meta), filemanager_(filemanager)
{
	if (task_meta_->cmd_args.size() != 2) {
		throw task_exception(
			"Wrong number of arguments. Required: 2, Actual: " + std::to_string(task_meta_->cmd_args.size()));
	}
}


fetch_task::~fetch_task()
{
}


std::shared_ptr<task_results> fetch_task::run()
{
	std::shared_ptr<task_results> result(new task_results());

	try {
		filemanager_->get_file(task_meta_->cmd_args[0], task_meta_->cmd_args[1]);
	} catch (fm_exception &e) {
		result->status = task_status::FAILED;
		result->error_message = std::string("Cannot fetch files. Error: ") + e.what();
	}

	return result;
}

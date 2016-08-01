#include "extract_task.h"
#include "../../archives/archivator.h"


extract_task::extract_task(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta_->cmd_args.size() != 2) {
		throw task_exception(
			"Wrong number of arguments. Required: 2, Actual: " + std::to_string(task_meta_->cmd_args.size()));
	}
}


extract_task::~extract_task()
{
}


std::shared_ptr<task_results> extract_task::run()
{
	try {
		archivator::decompress(task_meta_->cmd_args[0], task_meta_->cmd_args[1]);
		return std::shared_ptr<task_results>(new task_results());
	} catch (archive_exception &e) {
		throw task_exception(std::string("Cannot extract files. Error: ") + e.what());
	}
}

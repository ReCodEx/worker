#include "extract_task.h"
#include "../../archives/archivator.h"


extract_task::extract_task(size_t id,
	std::string task_id,
	size_t priority,
	bool fatal,
	const std::string &cmd,
	const std::vector<std::string> &arguments,
	const std::vector<std::string> &dependencies)
	: task_base(id, task_id, priority, fatal, dependencies, cmd, arguments)
{
	if (arguments_.size() != 2) {
		throw task_exception("Wrong number of arguments. Required: 2, Actual: " + arguments_.size());
	}
}


extract_task::~extract_task()
{
}


std::shared_ptr<task_results> extract_task::run()
{
	try {
		archivator::decompress(arguments_[0], arguments_[1]);
		return std::shared_ptr<task_results>(new task_results());
	} catch (archive_exception &e) {
		throw task_exception(std::string("Cannot extract files. Error: ") + e.what());
	}
}

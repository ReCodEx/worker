#include "extract_task.h"
#include "../../archives/archivator.h"


extract_task::extract_task(size_t id, std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::string &log,
			const std::vector<std::string> &dependencies)
	: task_base(id, task_id, priority, fatal, dependencies, cmd, arguments, log)
{
}


extract_task::~extract_task()
{
}


void extract_task::run()
{
	if (arguments_.size() != 2) {
		throw task_exception("Wrong number of arguments. Required: 2, Actual: " + arguments_.size());
	}
	try {
		archivator::decompress(arguments_[0], arguments_[1]);
	} catch (archive_exception &e) {
		throw task_exception(std::string("Cannot extract files. Error: ") + e.what());
	}
}

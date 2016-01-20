#include "rename_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


rename_task::rename_task(size_t id, std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::vector<std::string> &dependencies)
	: task_base(id, task_id, priority, fatal, dependencies, cmd, arguments)
{
	if (arguments_.size() != 2) {
		throw task_exception("Wrong number of arguments. Required: 2, Actual: " + arguments_.size());
	}
}


rename_task::~rename_task()
{
}


std::shared_ptr<task_results> rename_task::run()
{
	try {
		fs::rename(arguments_[0], arguments_[1]);
		return std::shared_ptr<task_results>(new task_results());
	} catch (fs::filesystem_error &e) {
		throw task_exception(std::string("Cannot rename files. Error: ") + e.what());
	}
}

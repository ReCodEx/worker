#include "cp_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


cp_task::cp_task(std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::string &log,
			const std::vector<std::string> &dependencies)
	: task_base(task_id, priority, fatal, cmd, arguments, log, dependencies)
{
}


cp_task::~cp_task()
{
}


void cp_task::run()
{
	if (arguments_.size() != 2) {
		throw task_exception("Wrong number of arguments. Required: 2, Actual: " + arguments_.size());
	}
	try {
		fs::copy(arguments_[0], arguments_[1]);
	} catch (fs::filesystem_error &e) {
		throw task_exception(std::string("Cannot copy files. Error: ") + e.what());
	}
}

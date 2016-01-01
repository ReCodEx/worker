#include "rm_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


rm_task::rm_task(std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::string &log,
			const std::vector<std::string> &dependencies)
	: task_base(task_id, priority, fatal, cmd, arguments, log, dependencies)
{
}


rm_task::~rm_task()
{
}


void rm_task::run()
{
	if (!arguments_.empty()) {
		throw task_exception("At least one argument required.");
	}

	//Try to delete all items
	bool result = true;
	for (auto &i : arguments_) {
		try {
			fs::remove_all(i);
		} catch (...) {
			result = false;
		}
	}

	//If anything cannot be deleted, throw exception
	if (!result) {
		throw task_exception("Cannot delete all directories.");
	}
}

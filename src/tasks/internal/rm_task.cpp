#include "rm_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


rm_task::rm_task(size_t id,
	std::string task_id,
	size_t priority,
	bool fatal,
	const std::string &cmd,
	const std::vector<std::string> &arguments,
	const std::vector<std::string> &dependencies)
	: task_base(id, task_id, priority, fatal, dependencies, cmd, arguments)
{
	if (arguments_.empty()) {
		throw task_exception("At least one argument required.");
	}
}


rm_task::~rm_task()
{
}


std::shared_ptr<task_results> rm_task::run()
{
	// Try to delete all items
	bool result = true;
	for (auto &i : arguments_) {
		try {
			fs::remove_all(i);
		} catch (...) {
			result = false;
		}
	}

	// If anything cannot be deleted, throw exception
	if (!result) {
		throw task_exception("Cannot delete all directories.");
	}
	return std::shared_ptr<task_results>(new task_results());
}

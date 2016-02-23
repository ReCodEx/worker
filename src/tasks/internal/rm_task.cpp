#include "rm_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


rm_task::rm_task(size_t id, task_metadata task_meta) : task_base(id, task_meta)
{
	if (task_meta_.cmd_args.empty()) {
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
	for (auto &i : task_meta_.cmd_args) {
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

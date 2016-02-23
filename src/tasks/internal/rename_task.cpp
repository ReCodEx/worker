#include "rename_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


rename_task::rename_task(size_t id, task_metadata task_meta) : task_base(id, task_meta)
{
	if (task_meta_.cmd_args.size() != 2) {
		throw task_exception("Wrong number of arguments. Required: 2, Actual: " + task_meta_.cmd_args.size());
	}
}


rename_task::~rename_task()
{
}


std::shared_ptr<task_results> rename_task::run()
{
	try {
		fs::rename(task_meta_.cmd_args[0], task_meta_.cmd_args[1]);
		return std::shared_ptr<task_results>(new task_results());
	} catch (fs::filesystem_error &e) {
		throw task_exception(std::string("Cannot rename files. Error: ") + e.what());
	}
}

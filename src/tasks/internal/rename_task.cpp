#include "rename_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


rename_task::rename_task(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta_->cmd_args.size() != 2) {
		throw task_exception(
			"Wrong number of arguments. Required: 2, Actual: " + std::to_string(task_meta_->cmd_args.size()));
	}
}


rename_task::~rename_task()
{
}


std::shared_ptr<task_results> rename_task::run()
{
	std::shared_ptr<task_results> result(new task_results());

	try {
		fs::rename(task_meta_->cmd_args[0], task_meta_->cmd_args[1]);
	} catch (fs::filesystem_error &e) {
		result->status = task_status::FAILED;
		result->error_message = std::string("Cannot rename files. Error: ") + e.what();
	}

	return result;
}

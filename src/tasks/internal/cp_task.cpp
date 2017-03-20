#include "cp_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


cp_task::cp_task(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta_->cmd_args.size() != 2) {
		throw task_exception(
			"Wrong number of arguments. Required: 2, Actual: " + std::to_string(task_meta_->cmd_args.size()));
	}
}


cp_task::~cp_task()
{
}


std::shared_ptr<task_results> cp_task::run()
{
	std::shared_ptr<task_results> result(new task_results());

	try {
		fs::copy(task_meta_->cmd_args[0], task_meta_->cmd_args[1]);
	} catch (fs::filesystem_error &e) {
		result->status = task_status::FAILED;
		result->error_message = std::string("Cannot copy files. Error: ") + e.what();
	}

	return result;
}

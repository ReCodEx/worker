#include "exists_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


exists_task::exists_task(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta_->cmd_args.size() < 2) { throw task_exception("At least two arguments required."); }
}


exists_task::~exists_task()
{
}


std::shared_ptr<task_results> exists_task::run()
{
	std::shared_ptr<task_results> result(new task_results());

	try {
		for (size_t i = 1; i < task_meta_->cmd_args.size(); ++i) {
			std::string file = task_meta_->cmd_args[i];
			if (!fs::exists(file)) {
				result->status = task_status::FAILED;
				result->error_message = "File/folder '" + file + "' cannot be found";
				result->output_stderr = task_meta_->cmd_args[0];
				break;
			}
		}
	} catch (fs::filesystem_error &e) {
		result->status = task_status::FAILED;
		result->error_message = std::string("Cannot check file/folder existance. Error: ") + e.what();
	}

	return result;
}

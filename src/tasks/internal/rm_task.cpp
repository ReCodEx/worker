#include "rm_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


rm_task::rm_task(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta_->cmd_args.empty()) { throw task_exception("At least one argument required."); }
}


rm_task::~rm_task()
{
}


std::shared_ptr<task_results> rm_task::run()
{
	std::shared_ptr<task_results> result(new task_results());

	// Try to delete all items
	for (auto &i : task_meta_->cmd_args) {
		try {
			fs::remove_all(i);
		} catch (...) {
			result->status = task_status::FAILED;
			result->error_message = "Cannot delete all directories.";
		}
	}

	return result;
}

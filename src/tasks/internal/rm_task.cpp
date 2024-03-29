#include "rm_task.h"
#include <filesystem>

namespace fs = std::filesystem;


rm_task::rm_task(std::size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta_->cmd_args.empty()) { throw task_exception("At least one argument required."); }
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

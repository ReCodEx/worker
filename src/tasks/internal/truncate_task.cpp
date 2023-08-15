#include <filesystem>
#include "truncate_task.h"

namespace fs = std::filesystem;

truncate_task::truncate_task(std::size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta->cmd_args.size() < 2) {
		throw task_exception(
			"Wrong number of arguments. Required: 2, Actual: " + std::to_string(task_meta_->cmd_args.size()));
	}
}


std::shared_ptr<task_results> truncate_task::run()
{
	auto results = std::make_shared<task_results>();

	fs::path file(task_meta_->cmd_args[0]);
	auto limit = read_task_arg<std::size_t>(task_meta_->cmd_args, 1, 128);
	limit *= 1024;

	if (fs::file_size(file) > limit) {
		std::error_code error_code;
		fs::resize_file(file, limit, error_code);

		if (error_code) { results->status = task_status::FAILED; }
	}

	return results;
}

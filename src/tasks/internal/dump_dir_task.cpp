#include <fstream>
#include <iostream>
#include "dump_dir_task.h"

dump_dir_task::dump_dir_task(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta) {
	if (task_meta->cmd_args.size() < 2) {
		throw task_exception(
			"Wrong number of arguments. Required: 2 (1 optional), Actual: "
			+ std::to_string(task_meta_->cmd_args.size()));
	}
}

dump_dir_task::~dump_dir_task() {

}

std::shared_ptr<task_results> dump_dir_task::run() {
	auto results = std::make_shared<task_results>();
	fs::path src_root(task_meta_->cmd_args[0]);
	fs::path dest_root(task_meta_->cmd_args[1]);
	auto limit_arg = task_meta_->cmd_args.size() >= 3 ? std::stoi(task_meta_->cmd_args[2]) : 0;

	if (limit_arg <= 0) {
		limit_arg = 128; // Default value
	}

	auto limit = static_cast<size_t>(limit_arg * 1024); // The argument is in kilobytes

	fs::recursive_directory_iterator directory_iterator(src_root), directory_iterator_end;
	std::vector<fs::path> paths(directory_iterator, directory_iterator_end);

	std::sort(paths.begin(), paths.end(), [] (const fs::path &a, const fs::path &b) {
		if (a == b) {
			return false;
		}

		if (fs::is_directory(a) && !fs::is_directory(b)) {
			return true;
		}

		if (fs::is_directory(b) && !fs::is_directory(a)) {
			return false;
		}

		if (fs::is_directory(a) && fs::is_directory(b)) {
			return a < b;
		}

		return fs::file_size(a) < fs::file_size(b);
	});

	for (auto &path: paths) {
		auto relative_path = fs::path(path.string().substr(src_root.string().size()));
		auto dest_path = dest_root / relative_path;

		if (fs::is_directory(path)) {
			fs::create_directories(dest_path);
		} else {
			size_t size = fs::file_size(path);
			if (size <= limit) {
				copy_file(path, dest_path);
				limit = size > limit ? 0 : limit - size;
			} else {
				std::ofstream placeholder(dest_path.string() + ".skipped", std::ios::out | std::ios::app);
				placeholder << " ";
				placeholder.close();
			}
		}
	}

	return results;
}

bool dump_dir_task::copy_file(const fs::path &src, const fs::path &dest) {
	boost::system::error_code error_code;
	fs::copy(src, dest, error_code);

	return error_code.value() == boost::system::errc::success;
}


#include <fstream>
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

	auto limit = read_task_arg<size_t>(task_meta_->cmd_args, 2, 128);
	limit *= 1024; // The argument is in kilobytes

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
				auto return_code = copy_file(path, dest_path);

				if (return_code.value() != boost::system::errc::success) {
					results->status = task_status::FAILED;
					results->error_message = "Copying `" + path.string() + "` to `"
								 + dest_path.string() +"` failed (error code "
								 + std::to_string(return_code.value()) + ")";
				}

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

boost::system::error_code dump_dir_task::copy_file(const fs::path &src, const fs::path &dest) {
	boost::system::error_code error_code;
	fs::copy(src, dest, error_code);

	return error_code;
}


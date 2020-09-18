#include <fstream>
#include "dump_dir_task.h"

dump_dir_task::dump_dir_task(std::size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta->cmd_args.size() < 3) {
		throw task_exception("Wrong number of arguments. Required: 3 (1+ optional), Actual: " +
			std::to_string(task_meta_->cmd_args.size()));
	}
}

std::shared_ptr<task_results> dump_dir_task::run()
{
	auto results = std::make_shared<task_results>();
	fs::path src_root(task_meta_->cmd_args[0]);
	fs::path dest_root(task_meta_->cmd_args[1]);

	auto limit = read_task_arg<std::size_t>(task_meta_->cmd_args, 2, 128);
	limit *= 1024; // The argument is in kilobytes

	fs::recursive_directory_iterator directory_iterator(src_root), directory_iterator_end;
	std::vector<fs::path> paths(directory_iterator, directory_iterator_end);

	// Drop directories from the paths
	paths.erase(std::remove_if(paths.begin(), paths.end(), [](const fs::path &path) { return fs::is_directory(path); }),
		paths.end());

	// Sort the paths by size (ascending order)
	std::sort(paths.begin(), paths.end(), [](const fs::path &a, const fs::path &b) {
		return fs::file_size(a) < fs::file_size(b);
	});

	for (auto &path : paths) {
		auto relative_path = fs::path(path.string().substr(src_root.string().size()));
		auto dest_path = dest_root / relative_path;

		if (!fs::exists(dest_path.parent_path())) {
			auto return_code = make_dirs(dest_path.parent_path());
			if (return_code.value() != boost::system::errc::success &&
				return_code.value() != boost::system::errc::file_exists) {
				results->status = task_status::FAILED;
				results->error_message = "Creating directory `" + dest_path.string() + "` failed (error code `" +
					std::to_string(return_code.value()) + "`)";
			}
		}

		std::size_t size = fs::file_size(path);
		if (size <= limit) {
			auto return_code = copy_file(path, dest_path);

			if (return_code.value() != boost::system::errc::success) {
				results->status = task_status::FAILED;
				results->error_message = "Copying `" + path.string() + "` to `" + dest_path.string() +
					"` failed (error code " + std::to_string(return_code.value()) + ")";
			}

			limit = size > limit ? 0 : limit - size;
		} else {
			std::ofstream placeholder(dest_path.string() + ".skipped", std::ios::out | std::ios::app);
			placeholder << " ";
			placeholder.close();
		}

		if (results->status != task_status::OK) { break; }
	}

	return results;
}

boost::system::error_code dump_dir_task::copy_file(const fs::path &src, const fs::path &dest)
{
	boost::system::error_code error_code;
	fs::copy(src, dest, error_code);

	return error_code;
}

boost::system::error_code dump_dir_task::make_dirs(const fs::path &path)
{
	boost::system::error_code error_code;
	fs::create_directories(path, error_code);

	return error_code;
}

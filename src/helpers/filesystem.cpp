#include "filesystem.h"

void helpers::copy_directory(const fs::path &src, const fs::path &dest)
{
	try {
		if (!fs::exists(src)) {
			throw filesystem_exception("Source directory does not exist");
		} else if (!fs::is_directory(src)) {
			throw filesystem_exception("Source directory is not a directory");
		} else if (fs::exists(dest)) {
			throw filesystem_exception("Destination directory exists");
		} else if (!fs::create_directories(dest)) {
			throw filesystem_exception("Destination directory cannot be created");
		}

		fs::directory_iterator endit;
		for (fs::directory_iterator it(src); it != endit; ++it) {
			if (fs::is_directory(it->status())) {
				helpers::copy_directory(it->path(), dest / it->path().filename());
			} else {
				fs::copy(it->path(), dest / it->path().filename());
			}
		}
	} catch (fs::filesystem_error &e) {
		throw filesystem_exception("Error in copying directories: " + std::string(e.what()));
	}

	return;
}

fs::path helpers::find_path_outside_sandbox(const std::string &inside,
	const std::string &sandbox_chdir,
	std::vector<std::tuple<std::string, std::string, sandbox_limits::dir_perm>> &bound_dirs)
{
	fs::path file_path = fs::path(inside);
	if (!file_path.has_root_directory()) {
		// relative path to chdir
		file_path = fs::path(sandbox_chdir) / file_path;
	}

	// absolute path in sandbox provided
	for (auto &dir : bound_dirs) {
		std::string sandbox_dir_bound = fs::path(std::get<1>(dir)).string();
		std::string file_path_string = file_path.string();

		if (file_path_string.find(sandbox_dir_bound) == 0) {
			std::string file_path_end = file_path_string.substr(sandbox_dir_bound.length());
			return fs::path(std::get<0>(dir)) / fs::path(file_path_end);
		}
	}
	return fs::path();
}

#include "filesystem.h"
#include <iostream>
#include <map>

/**
 * Try to find matching hardlink in hardlinks map. If src is found in the map, dest is filled with corresponding file.
 * @param hardlinks the hardlinks map (src -> dst)
 * @param src source path being looked up in hardlinks using equvalent func
 * @param dest output arg which is filled in case of success
 * @return true if the hardlink match is found
 */
bool find_matching_hardlink(std::map<fs::path, fs::path> &hardlinks, const fs::path &src, fs::path &dest)
{
	for (auto& [s, d] : hardlinks) {
		if (fs::equivalent(s, src)) { // both paths point to the same data on the disk (same hardlinks)
			dest = d;
			return true;
		}
	}
	return false;
}

void copy_diretory_internal(const fs::path &src, const fs::path &dest, bool skip_symlinks, std::map<fs::path, fs::path> &hardlinks)
{
	try {
		// routine checks
		if (!fs::exists(src)) {
			throw helpers::filesystem_exception(
				"helpers::copy_directory: Source directory does not exist '" + src.string() + "'");
		}

		if (skip_symlinks && fs::is_symlink(src)) {
			return;
		}
		
		if (!fs::is_directory(fs::symlink_status(src))) {
			throw helpers::filesystem_exception(
				"helpers::copy_directory: Source directory is not a directory '" + src.string() + "'");
		}
		
		if (!fs::exists(dest) && !fs::create_directories(dest)) {
			throw helpers::filesystem_exception(
				"helpers::copy_directory: Destination directory cannot be created '" + dest.string() + "'");
		}

		// proceed with copying
		fs::directory_iterator endit;
		for (fs::directory_iterator it(src); it != endit; ++it) {
			auto srcPath = it->path();
			auto destPath = dest / it->path().filename();

			if (skip_symlinks && fs::is_symlink(srcPath)) {
				continue;
			}

			if (fs::is_directory(it->symlink_status())) {
				copy_diretory_internal(srcPath, destPath, skip_symlinks, hardlinks);
			} else {
				// a file may be either copied or hardlinked
				if (!fs::is_symlink(srcPath) && fs::hard_link_count(srcPath) > 1) {
					fs::path destPathHardlink;
					if (find_matching_hardlink(hardlinks, it->path(), destPathHardlink)) {
						// another file refering to the same data already exists in dest directory
						fs::create_hard_link(destPathHardlink, destPath);
						continue; // move to next file, hardlink replaced copying
					} else {
						// this is the first time we encoutered this data, lets register them in hardlinks map
						hardlinks.emplace(std::make_pair(it->path(), destPath));
					}
				}

				// no hardlink created, lets proceed with copying
				fs::copy(it->path(), destPath);
			}
		}
	} catch (fs::filesystem_error &e) {
		throw helpers::filesystem_exception("helpers::copy_directory: Error in copying directories: " + std::string(e.what()));
	}

	return;
}

void helpers::copy_directory(const fs::path &src, const fs::path &dest, bool skip_symlinks)
{
	/*
	 * Hardlinks map provide mapping between files in src and dest which have been harlinked.
	 * Everytime a file with > 1 hardlink count is copied from src to dest, it is registered here.
	 * When files with > 1 hardlinks are encountered, this map is searched and if match is found
	 * the new file is hardlinked inside dest instead of coping it from src.
	 */
	std::map<fs::path, fs::path> hardlinks;
	::copy_diretory_internal(src, dest, skip_symlinks, hardlinks);
}

fs::path helpers::normalize_path(const fs::path &path)
{
	// prepare root and path chunks
	fs::path root = path.root_path();
	std::vector<fs::path> path_arr(path.begin(), path.end());

	// process path chunks
	std::vector<fs::path> result_arr;
	for (auto &chunk : path_arr) {
		if (chunk.string() == ".") {
			continue;
		} else if (chunk.string() == "..") {
			if (result_arr.empty() || (!root.empty() && result_arr.back().string() == root)) {
				// invalid path, either there is no more elements, or root was hit
				return fs::path();
			}
			result_arr.pop_back();
			continue;
		}

		result_arr.push_back(chunk);
	}

	// construct resulting path
	fs::path result;
	for (auto &chunk : result_arr) { result /= chunk; }
	return result;
}

bool helpers::check_relative(const fs::path &path)
{
	if (path.is_absolute()) { return false; }

	std::vector<fs::path> path_arr(path.begin(), path.end());
	for (auto &chunk : path_arr) {
		if (chunk.string() == "..") { return false; }
	}

	return true;
}

fs::path helpers::find_path_outside_sandbox(const std::string &inside_path,
	const std::string &sandbox_chdir,
	std::vector<std::tuple<std::string, std::string, sandbox_limits::dir_perm>> &bound_dirs,
	const std::string &source_dir)
{
	auto file_path = fs::path(inside_path);
	if (!file_path.has_root_directory()) {
		// relative path to chdir, chdir should be absolute
		file_path = fs::path(sandbox_chdir) / file_path;
	} else {
		// for absolute paths remove /box prefix if any
		std::string box_path = "/box";
		if (inside_path.find(box_path) == 0) { file_path = fs::path(inside_path.substr(box_path.length())); }
	}
	file_path = normalize_path(file_path);
	auto file_path_string = file_path.string();

	// try to find the file in main source directory
	auto source_path = fs::path(source_dir) / file_path.relative_path();
	if (fs::exists(source_path)) { return source_path; }

	// try to find the file in sandbox bound directories (absolute path in sandbox provided)
	for (auto &dir : bound_dirs) {
		std::string sandbox_dir_string = normalize_path(fs::path(std::get<1>(dir))).string();

		if (file_path_string.find(sandbox_dir_string) == 0) {
			std::string file_path_end = file_path_string.substr(sandbox_dir_string.length());
			return fs::path(std::get<0>(dir)) / fs::path(file_path_end).relative_path();
		}
	}

	// not found
	return fs::path();
}

#include "cp_task.h"
#include "helpers/string_utils.h"
#include "helpers/filesystem.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


cp_task::cp_task(std::size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta_->cmd_args.size() != 2) {
		throw task_exception(
			"Wrong number of arguments. Required: 2, Actual: " + std::to_string(task_meta_->cmd_args.size()));
	}
}


std::shared_ptr<task_results> cp_task::run()
{
	std::shared_ptr<task_results> result(new task_results());

	// parse the input path and separate the last part
	fs::path input(task_meta_->cmd_args[0]);
	auto filename_matcher = input.filename().string();
	auto base_dir = input.remove_filename();
	auto pattern = helpers::wildcards_regex(filename_matcher);

	// parse output path and check if it is existing directory or not
	fs::path output(task_meta_->cmd_args[1]);
	bool output_is_dir = fs::is_directory(output);

	if (!fs::exists(base_dir)) {
		result->status = task_status::FAILED;
		result->error_message = "Source directory '" + base_dir.string() + "' does not exist";
		return result;
	}

	// go through all items in base directory and copy matching items to result location
	// TODO: replace to range loop when boost is upgraded
	fs::directory_iterator end_itr;
	for (fs::directory_iterator item(base_dir); item != end_itr; ++item) {
		if (regex_match(item->path().filename().string(), pattern)) {
			auto target = output_is_dir ? (output / item->path().filename()) : output;
			if (fs::is_directory(fs::symlink_status(item->path()))) {
				helpers::copy_directory(item->path(), target);
			} else {
				boost::system::error_code error_code;
				fs::copy(item->path(), target, error_code);
				if (error_code.value() != boost::system::errc::success) {
					result->status = task_status::FAILED;
					result->error_message = std::string("Cannot copy files. Error: ") + error_code.message();
					break;
				}
			}
		}
	}

	return result;
}

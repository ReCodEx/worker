#include "cp_task.h"
#include <regex>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace {

void replace_substring(std::string &data, const std::string &from, const std::string &to)
{
	std::size_t pos = data.find(from);
	while(pos != std::string::npos) {
		data.replace(pos, from.size(), to);
		pos = data.find(from, pos + to.size());
	}
}

void escape_regex(std::string &regex)
{
	replace_substring(regex, "\\", "\\\\");
	replace_substring(regex, "^", "\\^");
	replace_substring(regex, ".", "\\.");
	replace_substring(regex, "$", "\\$");
	replace_substring(regex, "|", "\\|");
	replace_substring(regex, "(", "\\(");
	replace_substring(regex, ")", "\\)");
	replace_substring(regex, "[", "\\[");
	replace_substring(regex, "]", "\\]");
	replace_substring(regex, "*", "\\*");
	replace_substring(regex, "+", "\\+");
	replace_substring(regex, "?", "\\?");
	replace_substring(regex, "/", "\\/");
}

std::regex wildcards_regex(std::string wildcard_pattern)
{
	// Escape all regex special chars
	escape_regex(wildcard_pattern);

	// Convert chars '*?' back to their regex equivalents
	replace_substring(wildcard_pattern, "\\?", ".");
	replace_substring(wildcard_pattern, "\\*", ".*");

	return std::regex(wildcard_pattern);
}

} // namespace


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

	// Cannot use this normal variant due to bug in boost which
	// causes calling abort() instead of throwing an exception
	// try {
	// 	fs::copy(task_meta_->cmd_args[0], task_meta_->cmd_args[1]);
	// } catch (fs::filesystem_error &e) {
	// 	result->status = task_status::FAILED;
	// 	result->error_message = std::string("Cannot copy files. Error: ") + e.what();
	// }

	// Instead, following version seems to work fine

	// parse the input path and separate the last part
	fs::path input(task_meta_->cmd_args[0]);
	auto filename_matcher = input.filename().string();
	auto base_dir = input.remove_filename();
	auto pattern = wildcards_regex(filename_matcher);

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
			boost::system::error_code error_code;
			fs::copy(item->path(), target, error_code);
			if (error_code.value() != boost::system::errc::success) {
				result->status = task_status::FAILED;
				result->error_message = std::string("Cannot copy files. Error: ") + error_code.message();
				break;
			}
		}
	}

	return result;
}

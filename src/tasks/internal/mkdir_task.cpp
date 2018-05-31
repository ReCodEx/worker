#include "mkdir_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


mkdir_task::mkdir_task(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
{
	if (task_meta_->cmd_args.empty()) { throw task_exception("At least one argument required."); }
}


mkdir_task::~mkdir_task()
{
}


std::shared_ptr<task_results> mkdir_task::run()
{
	std::shared_ptr<task_results> result(new task_results());

	try {
		for (auto &i : task_meta_->cmd_args) {
			fs::create_directories(i);
			fs::permissions(i, fs::add_perms | fs::group_write | fs::others_write);
		}
	} catch (fs::filesystem_error &e) {
		// Remove already created directories
		for (auto &i : task_meta_->cmd_args) {
			try {
				fs::remove_all(i);
			} catch (...) {
			}
		}

		result->status = task_status::FAILED;
		result->error_message = std::string("Cannot create all directories. Error: ") + e.what();
	}

	return result;
}

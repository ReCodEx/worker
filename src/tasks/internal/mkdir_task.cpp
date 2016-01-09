#include "mkdir_task.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


mkdir_task::mkdir_task(size_t id, std::string task_id, size_t priority, bool fatal, const std::string &cmd,
			const std::vector<std::string> &arguments, const std::string &log,
			const std::vector<std::string> &dependencies)
	: task_base(id, task_id, priority, fatal, dependencies, cmd, arguments, log)
{
	if (arguments_.empty()) {
		throw task_exception("At least one argument required.");
	}
}


mkdir_task::~mkdir_task()
{
}


void mkdir_task::run()
{
	try {
		for (auto &i : arguments_) {
			fs::create_directories(i);
		}
	} catch (fs::filesystem_error &e) {
		//Remove already created directories
		for (auto &i : arguments_) {
			try {
				fs::remove_all(i);
			} catch (...) {}
		}
		throw task_exception(std::string("Cannot create all directories. Error: ") + e.what());
	}
}

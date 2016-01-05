#include "external_task.h"

external_task::external_task(size_t id, const std::string &task_id, size_t priority, bool fatal, const std::string &log,
							 const std::vector<std::string> &dependencies,
							 const std::string &binary, const std::vector<std::string> &arguments,
							 const std::string &sandbox, sandbox_limits limits)
	: task_base(id, task_id, priority, fatal, dependencies, binary, arguments, log),
	  cmd_(binary), sandbox_id_(sandbox), limits_(limits)
{
	sandbox_init();
}

external_task::~external_task()
{}

void external_task::sandbox_init()
{
	bool constructed = false;

#ifndef _WIN32
	if (sandbox_id_ == "isolate") {
		// TODO: isolate uid not given (its constant 1)
		sandbox_ = std::make_shared<isolate_sandbox>(limits_, 1);
		constructed = true;
	}
#endif

	if (constructed == false) {
		throw task_exception("Unknown sandbox type: " + sandbox_id_);
	}
}

void external_task::run()
{
	sandbox_->run(cmd_, arguments_);
	return;
}

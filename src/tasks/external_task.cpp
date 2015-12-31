#include "external_task.h"

external_task::external_task(const std::string &binary, const std::vector<std::string> &arguments,
							 const std::string &sandbox, sandbox_limits limits)
	: task_base("", 0, false, "", "", std::vector<std::string>()),
	  cmd_(binary), args_(arguments), sandbox_id_(sandbox), limits_(limits)
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
	sandbox_->run(cmd_, args_);
	return;
}

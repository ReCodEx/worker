#include "external_task.h"

external_task::external_task(size_t worker_id, size_t id, const std::string &task_id, size_t priority,
							 bool fatal, const std::vector<std::string> &dependencies,
							 const std::string &binary, const std::vector<std::string> &arguments,
							 const std::string &sandbox_id, sandbox_limits limits, std::shared_ptr<spdlog::logger> logger)
	: task_base(id, task_id, priority, fatal, dependencies, binary, arguments),
	  worker_id_(worker_id), cmd_(binary), sandbox_id_(sandbox_id), limits_(limits), logger_(logger)
{
	sandbox_check();
}

external_task::~external_task()
{}

void external_task::sandbox_check()
{
	bool found = false;

#ifndef _WIN32
	if (sandbox_id_ == "isolate") {
		found = true;
	}
#endif

	if (found == false) {
		throw task_exception("Unknown sandbox type: " + sandbox_id_);
	}
}

void external_task::sandbox_init()
{
#ifndef _WIN32
	if (sandbox_id_ == "isolate") {
		sandbox_ = std::make_shared<isolate_sandbox>(limits_, worker_id_, -1, logger_);
	}
#endif
}

void external_task::sandbox_fini()
{
	sandbox_ = nullptr;
}

std::shared_ptr<task_results> external_task::run()
{
	sandbox_init();

	// TODO: only temporary solution, should be removed
	if (sandbox_ == nullptr) { return nullptr; }

	auto res = std::shared_ptr<task_results>(new task_results());
	res->sandbox_status = std::unique_ptr<sandbox_results>(new sandbox_results(sandbox_->run(cmd_, arguments_)));

	sandbox_fini();

	return res;
}

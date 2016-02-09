#include "external_task.h"

external_task::external_task(const create_params &data)
	: task_base(data.id, data.task_id, data.priority, data.fatal, data.dependencies, data.binary, data.arguments),
	  worker_id_(data.worker_id), cmd_(data.binary), sandbox_id_(data.sandbox_id), limits_(data.limits),
	  logger_(data.logger), temp_dir_(data.temp_dir)
{
	sandbox_check();
}

external_task::~external_task()
{}

void external_task::sandbox_check()
{
	bool found = false;

	if (sandbox_id_ == "fake") {
		found = true;
	}
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
	if (sandbox_id_ == "fake") {
		sandbox_ = std::make_shared<fake_sandbox>();
	}
#ifndef _WIN32
	if (sandbox_id_ == "isolate") {
		sandbox_ = std::make_shared<isolate_sandbox>(limits_, worker_id_, temp_dir_, -1, logger_);
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

sandbox_limits external_task::get_limits()
{
	return limits_;
}

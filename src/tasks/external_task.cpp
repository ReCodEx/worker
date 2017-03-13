#include "external_task.h"
#include "../sandbox/isolate_sandbox.h"

external_task::external_task(const create_params &data)
	: task_base(data.id, data.task_meta), worker_id_(data.worker_id), sandbox_(nullptr),
	  sandbox_config_(data.sandbox_conf), limits_(data.limits), logger_(data.logger), temp_dir_(data.temp_dir)
{
	sandbox_check();
}

external_task::~external_task()
{
}

void external_task::sandbox_check()
{
	bool found = false;

	if (task_meta_->sandbox == nullptr) {
		throw task_exception("No sandbox configuration provided.");
	}

#ifndef _WIN32
	if (task_meta_->sandbox->name == "isolate") {
		found = true;
	}
#endif

	if (found == false) {
		throw task_exception("Unknown sandbox type: " + task_meta_->sandbox->name);
	}
}

void external_task::sandbox_init()
{
#ifndef _WIN32
	if (task_meta_->sandbox->name == "isolate") {
		sandbox_ = std::make_shared<isolate_sandbox>(sandbox_config_, *limits_, worker_id_, temp_dir_, logger_);
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
	if (sandbox_ == nullptr) {
		return nullptr;
	}

	auto res = std::shared_ptr<task_results>(new task_results());
	res->sandbox_status =
		std::unique_ptr<sandbox_results>(new sandbox_results(sandbox_->run(task_meta_->binary, task_meta_->cmd_args)));

	sandbox_fini();

	// Check if sandbox ran successfully, else report error
	if (res->sandbox_status->status != isolate_status::OK) {
		throw task_exception("Sandboxed program failed: " + res->sandbox_status->message);
	}

	return res;
}

std::shared_ptr<sandbox_limits> external_task::get_limits()
{
	return limits_;
}

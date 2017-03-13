#include "external_task.h"
#include "../sandbox/isolate_sandbox.h"
#include "../helpers/string_utils.h"
#include <fstream>
#include <algorithm>

external_task::external_task(const create_params &data)
	: task_base(data.id, data.task_meta), worker_config_(data.worker_conf), sandbox_(nullptr),
	  sandbox_config_(data.sandbox_conf), limits_(data.limits), logger_(data.logger), temp_dir_(data.temp_dir)
{
	if (worker_config_ == nullptr) {
		throw task_exception("No worker configuration provided.");
	}

	if (sandbox_config_ == nullptr) {
		throw task_exception("No sandbox configuration provided.");
	}

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
		sandbox_ = std::make_shared<isolate_sandbox>(
			sandbox_config_, *limits_, worker_config_->get_worker_id(), temp_dir_, logger_);
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

	// initialize output from stdout and stderr
	results_output_init();

	auto res = std::shared_ptr<task_results>(new task_results());
	res->sandbox_status =
		std::unique_ptr<sandbox_results>(new sandbox_results(sandbox_->run(task_meta_->binary, task_meta_->cmd_args)));

	// get output from stdout and stderr
	res->sandbox_status->output = get_results_output();

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

void external_task::results_output_init()
{
	if (sandbox_config_->output) {
		std::string random = helpers::random_alphanum_string(10);
		if (sandbox_config_->std_output == "") {
			remove_stdout_ = true;
			sandbox_config_->std_output = task_meta_->task_id + "." + random + ".output.stdout";
		}

		if (sandbox_config_->std_error == "") {
			remove_stderr_ = true;
			sandbox_config_->std_error = task_meta_->task_id + "." + random + ".output.stderr";
		}
	}
}

std::string external_task::get_results_output()
{
	std::string result;

	if (sandbox_config_->output) {
		size_t count = worker_config_->get_max_output_length();
		std::ifstream stdout(sandbox_config_->std_output);
		std::ifstream stderr(sandbox_config_->std_error);
		std::copy_n(std::istreambuf_iterator<char>(stdout), count, std::back_inserter(result));
		std::copy_n(std::istreambuf_iterator<char>(stderr), count, std::back_inserter(result));

		// delete produced files
		try {
			if (remove_stdout_) {
				fs::remove(sandbox_config_->std_output);
			}
			if (remove_stderr_) {
				fs::remove(sandbox_config_->std_error);
			}
		} catch (fs::filesystem_error &e) {
			logger_->warn("Temporary sandbox output files not cleaned properly: {}", e.what());
		}
	}

	return result;
}

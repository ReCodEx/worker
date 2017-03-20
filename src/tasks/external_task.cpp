#include "external_task.h"
#include "../sandbox/isolate_sandbox.h"
#include "../helpers/string_utils.h"
#include <fstream>
#include <algorithm>

external_task::external_task(const create_params &data)
	: task_base(data.id, data.task_meta), worker_config_(data.worker_conf), sandbox_(nullptr),
	  sandbox_config_(data.task_meta->sandbox), limits_(data.limits), logger_(data.logger), temp_dir_(data.temp_dir),
	  source_dir_(data.source_path), working_dir_(data.working_path)
{
	if (worker_config_ == nullptr) {
		throw task_exception("No worker configuration provided.");
	}

	if (limits_ == nullptr) {
		throw task_exception("No limits provided.");
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
			std::string stdout_file = task_meta_->task_id + "." + random + ".output.stdout";
			sandbox_config_->std_output = (working_dir_ / fs::path(stdout_file)).string();
		}

		if (sandbox_config_->std_error == "") {
			remove_stderr_ = true;
			std::string stderr_file = task_meta_->task_id + "." + random + ".output.stderr";
			sandbox_config_->std_error = (working_dir_ / fs::path(stderr_file)).string();
		}
	}
}

fs::path external_task::find_path_outside_sandbox(std::string file)
{
	fs::path file_path = fs::path(file);
	fs::path sandbox_dir = file_path.parent_path();
	for (auto &dir : limits_->bound_dirs) {
		fs::path sandbox_dir_bound = fs::path(std::get<1>(dir));
		if (sandbox_dir_bound == sandbox_dir) {
			return fs::path(std::get<0>(dir)) / file_path.filename();
		}
	}
	return fs::path();
}

std::string external_task::get_results_output()
{
	std::string result;

	if (sandbox_config_->output) {
		size_t count = worker_config_->get_max_output_length();
		std::string result_stdout(count, 0);
		std::string result_stderr(count, 0);

		// files were outputted inside sandbox, so we have to find path outside sandbox
		fs::path stdout_file_path = find_path_outside_sandbox(sandbox_config_->std_output);
		fs::path stderr_file_path = find_path_outside_sandbox(sandbox_config_->std_error);

		// open and read files
		std::ifstream std_out(stdout_file_path.string());
		std::ifstream std_err(stderr_file_path.string());
		std_out.read(&result_stdout[0], count);
		std_err.read(&result_stderr[0], count);

		// if there was something in one of the files, write it to the result
		if (std_out.gcount() != 0 || std_err.gcount() != 0) {
			result = result_stdout.substr(0, std_out.gcount()) + result_stderr.substr(0, std_err.gcount());
		}

		// delete produced files
		try {
			if (remove_stdout_) {
				fs::remove(stdout_file_path);
			}
			if (remove_stderr_) {
				fs::remove(stderr_file_path);
			}
		} catch (fs::filesystem_error &e) {
			logger_->warn("Temporary sandbox output files not cleaned properly: {}", e.what());
		}
	}

	return result;
}

#include "progress_callback.h"
#include "../helpers/zmq_socket.h"
#include "../helpers/logger.h"
#include "../connection_proxy.h"

progress_callback::progress_callback(std::shared_ptr<zmq::context_t> context, std::shared_ptr<spdlog::logger> logger)
	: socket_(*context, ZMQ_PAIR), command_("progress"), connected_(false), logger_(logger)
{
	if (logger_ == nullptr) {
		logger_ = helpers::create_null_logger();
	}
}

void progress_callback::connect()
{
	if (!connected_) {
		socket_.connect("inproc://" + PROGRESS_SOCKET_ID);
		connected_ = true;
	}
}

void progress_callback::send_job_status(
	const std::string &func_name, const std::string &job_id, const std::string &job_status)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, job_status};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn("progress_callback: call of {} failed", func_name);
		logger_->warn("    -> job_id: {}", job_id);
	}
}

void progress_callback::job_archive_downloaded(const std::string &job_id)
{
	send_job_status("submission_downloaded", job_id, "DOWNLOADED");
}

void progress_callback::job_build_failed(const std::string &job_id)
{
	send_job_status("submission_failed", job_id, "FAILED");
}

void progress_callback::job_finished(const std::string &job_id)
{
	send_job_status("submission_finished", job_id, "FINISHED");
}

void progress_callback::job_results_uploaded(const std::string &job_id)
{
	send_job_status("job_results_uploaded", job_id, "UPLOADED");
}

void progress_callback::job_started(const std::string &job_id)
{
	send_job_status("job_started", job_id, "STARTED");
}

void progress_callback::job_ended(const std::string &job_id)
{
	send_job_status("job_ended", job_id, "ENDED");
}

void progress_callback::job_aborted(const std::string &job_id)
{
	send_job_status("job_aborted", job_id, "ABORTED");
}

void progress_callback::send_task_status(
	const std::string &func_name, const std::string &job_id, const std::string &task_id, const std::string &task_status)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, "TASK", task_id, task_status};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn("progress_callback: call of {} failed", func_name);
		logger_->warn("    -> job_id: {}; task_id: {}", job_id, task_id);
	}
}

void progress_callback::task_completed(const std::string &job_id, const std::string &task_id)
{
	send_task_status("task_completed", job_id, task_id, "COMPLETED");
}

void progress_callback::task_failed(const std::string &job_id, const std::string &task_id)
{
	send_task_status("task_failed", job_id, task_id, "FAILED");
}

void progress_callback::task_skipped(const std::string &job_id, const std::string &task_id)
{
	send_task_status("task_skipped", job_id, task_id, "SKIPPED");
}

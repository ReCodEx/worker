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

void progress_callback::submission_downloaded(const std::string &job_id)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, "DOWNLOADED"};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn() << "progress_callback: call of submission_downloaded failed";
		logger_->warn() << "    -> job_id: " << job_id;
	}
}

void progress_callback::job_results_uploaded(const std::string &job_id)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, "UPLOADED"};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn() << "progress_callback: call of job_results_uploaded failed";
		logger_->warn() << "    -> job_id: " << job_id;
	}
}

void progress_callback::job_started(const std::string &job_id)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, "STARTED"};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn() << "progress_callback: call of job_started failed";
		logger_->warn() << "    -> job_id: " << job_id;
	}
}

void progress_callback::job_ended(const std::string &job_id)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, "ENDED"};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn() << "progress_callback: call of job_ended failed";
		logger_->warn() << "    -> job_id: " << job_id;
	}
}

void progress_callback::task_completed(const std::string &job_id, const std::string &task_id)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, "TASK", task_id, "COMPLETED"};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn() << "progress_callback: call of task_completed failed";
		logger_->warn() << "    -> job_id: " << job_id << "; task_id: " << task_id;
	}
}

void progress_callback::task_failed(const std::string &job_id, const std::string &task_id)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, "TASK", task_id, "FAILED"};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn() << "progress_callback: call of task_failed failed";
		logger_->warn() << "    -> job_id: " << job_id << "; task_id: " << task_id;
	}
}

void progress_callback::task_skipped(const std::string &job_id, const std::string &task_id)
{
	try {
		connect();
		std::vector<std::string> msg = {command_, job_id, "TASK", task_id, "SKIPPED"};
		helpers::send_through_socket(socket_, msg);
	} catch (...) {
		logger_->warn() << "progress_callback: call of task_skipped failed";
		logger_->warn() << "    -> job_id: " << job_id << "; task_id: " << task_id;
	}
}

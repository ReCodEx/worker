#include "job_receiver.h"
#include "../connection_proxy.h"
#include "../eval_request.h"
#include "../eval_response.h"

job_receiver::job_receiver(
	zmq::context_t &context, std::shared_ptr<job_evaluator> evaluator, std::shared_ptr<spdlog::logger> logger)
	: socket_(context, ZMQ_PAIR), evaluator_(evaluator), logger_(logger)
{
	if (logger == nullptr) {
		// Create logger manually to avoid global registration of logger
		auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
		logger_ = std::make_shared<spdlog::logger>("job_receiver_nolog", sink);
	}
}

void job_receiver::start_receiving()
{
	socket_.connect("inproc://" JOB_SOCKET_ID);

	while (true) {
		try {
			zmq::message_t msg;

			logger_->info() << "Job-receiver: Waiting for incomings requests...";
			socket_.recv(&msg);
			std::string type(static_cast<char *>(msg.data()), msg.size());

			if (type == "eval") {
				if (!msg.more()) {
					continue;
				}

				socket_.recv(&msg);
				std::string id(static_cast<char *>(msg.data()), msg.size());

				if (!msg.more()) {
					continue;
				}

				socket_.recv(&msg);
				std::string job_url(static_cast<char *>(msg.data()), msg.size());

				if (!msg.more()) {
					continue;
				}

				socket_.recv(&msg);
				std::string result_url(static_cast<char *>(msg.data()), msg.size());

				logger_->info() << "Job-receiver: Job evaluating request received.";

				eval_response response = evaluator_->evaluate(eval_request(id, job_url, result_url));
				std::string response_command("eval_finished");

				socket_.send(response_command.c_str(), response_command.size(), ZMQ_SNDMORE);
				socket_.send(response.job_id.c_str(), response.job_id.size(), ZMQ_SNDMORE);
				socket_.send(response.result.c_str(), response.result.size(), 0);

				logger_->info() << "Job-receiver: Job evaluated and respond sent.";
			}

			while (msg.more()) {
				socket_.recv(&msg);
			}
		} catch (zmq::error_t &e) {
			logger_->emerg() << "Job-receiver error: " << e.what();
			break;
		}
	}
}

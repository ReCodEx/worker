#include "job_receiver.h"
#include "../connection_proxy.h"
#include "../eval_request.h"
#include "../eval_response.h"
#include "../helpers/zmq_socket.h"

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
	socket_.connect("inproc://" + JOB_SOCKET_ID);

	while (true) {
		logger_->info() << "Job-receiver: Waiting for incomings requests...";

		std::vector<std::string> message;
		bool terminate;
		if (!helpers::recv_from_socket(socket_, message, &terminate)) {
			if (terminate) {
				break;
			}
			logger_->warn() << "Job-receiver: failed to receive message. Skipping...";
			continue;
		}

		if (message.size() == 4 && message[0] == "eval") {
			logger_->info() << "Job-receiver: Job evaluating request received.";

			eval_response response = evaluator_->evaluate(eval_request(message[1], message[2], message[3]));
			std::vector<std::string> reply = { "done", response.job_id, response.result };

			helpers::send_through_socket(socket_, reply);
			logger_->info() << "Job-receiver: Job evaluated and respond sent.";
		}
	}
}

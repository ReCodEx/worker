#include "job_receiver.h"
#include "connection_proxy.h"
#include "eval_request.h"

job_receiver::job_receiver (zmq::context_t &context, std::shared_ptr<job_evaluator> evaluator) :
	socket_(context, ZMQ_PAIR), evaluator_(evaluator)
{
}

void job_receiver::start_receiving ()
{
	socket_.connect("inproc://" JOB_SOCKET_ID);

	while (true) {
		zmq::message_t msg;

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

			evaluator_->evaluate(eval_request(id, job_url, result_url));
		}

		while (msg.more()) {
			socket_.recv(&msg);
		}
	}
}

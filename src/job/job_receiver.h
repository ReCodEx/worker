#ifndef CODEX_WORKER_JOB_RECEIVER_H
#define CODEX_WORKER_JOB_RECEIVER_H


#include <zmq.hpp>
#include <vector>
#include <string>
#include "job_evaluator.h"
#include "../commands/command_holder.h"

/**
 * Job receiver handles incoming requests from broker_connection and
 * hand it over to job evaluator. It also sends back response from evaluator.
 */
class job_receiver
{
private:
	zmq::socket_t socket_;
	std::shared_ptr<job_evaluator> evaluator_;
	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<command_holder<job_client_context>> commands_;

public:
	/**
	 * Construct job receiver and fill it with given data.
	 * @param context
	 * @param evaluator evaluator which will evaluate received tasks
	 * @param logger pointer to logging class
	 */
	job_receiver(std::shared_ptr<zmq::context_t> context,
		std::shared_ptr<job_evaluator> evaluator,
		std::shared_ptr<spdlog::logger> logger);

	/**
	 * Receive jobs from an inproc socket and pass them to the evaluator
	 * Blocks execution until interrupted.
	 */
	void start_receiving();
};


#endif // CODEX_WORKER_JOB_RECEIVER_H

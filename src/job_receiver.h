#ifndef CODEX_WORKER_JOB_RECEIVER_H
#define CODEX_WORKER_JOB_RECEIVER_H


#include <zmq.hpp>
#include "job_evaluator.h"

class job_receiver {
private:
	zmq::socket_t socket_;
	std::shared_ptr<job_evaluator> evaluator_;

public:
	job_receiver (zmq::context_t &context, std::shared_ptr<job_evaluator> evaluator);

	/**
	 * Receive jobs from an inproc socket and pass them to the evaluator
	 * Blocks execution until interrupted.
	 */
	void start_receiving ();
};


#endif //CODEX_WORKER_JOB_RECEIVER_H

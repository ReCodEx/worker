#ifndef RECODEX_WORKER_EVAL_RESPONSE_H
#define RECODEX_WORKER_EVAL_RESPONSE_H

#include <string>

/**
 * A structure that contains the result of an evaluation.
 */
struct eval_response {
	/** ID of the job */
	const std::string job_id;

	/** Textual representation of the evaluation result */
	const std::string result;

	/** Description of error if needed */
	const std::string message;

	/**
	 * Classical initialization constructor.
	 * @param job_id ID of the job
	 * @param result a textual representation of the evaluation result
	 * @param message if execution failed then this will contain error message
	 */
	eval_response(std::string job_id, std::string result, std::string message = "")
		: job_id(job_id), result(result), message(message)
	{
	}
};

#endif // RECODEX_WORKER_EVAL_RESPONSE_H

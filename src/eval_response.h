#ifndef RECODEX_WORKER_EVAL_RESPONSE_H
#define RECODEX_WORKER_EVAL_RESPONSE_H

#include <string>

/**
 * A structure that contains the result of an evaluation
 */
struct eval_response {
	/** ID of the job */
	const std::string job_id;

	/** a rtextual representation of the evaluation result */
	const std::string result;

	/**
	 * @param job_id ID of the job
	 * @param result a textual representation of the evaluation result
	 */
	eval_response(std::string job_id, std::string result) : job_id(job_id), result(result)
	{
	}
};

#endif // RECODEX_WORKER_EVAL_RESPONSE_H

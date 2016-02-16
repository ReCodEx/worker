#ifndef CODEX_WORKER_EVAL_RESPONSE_H
#define CODEX_WORKER_EVAL_RESPONSE_H

#include <string>

/**
 * A structure that contains the result of an evaluation
 */
struct eval_response {
	const std::string job_id;
	const std::string result;

	eval_response(std::string job_id, std::string result) : job_id(job_id), result(result)
	{
	}
};

#endif // CODEX_WORKER_EVAL_RESPONSE_H

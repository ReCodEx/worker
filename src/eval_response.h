#ifndef RECODEX_WORKER_EVAL_RESPONSE_H
#define RECODEX_WORKER_EVAL_RESPONSE_H

#include <string>

/**
 * A structure that contains the result of an evaluation.
 */
struct eval_response {
public:
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
	eval_response(const std::string &job_id, const std::string &result, const std::string &message = "")
		: job_id(job_id), result(result), message(message)
	{
	}
};

/**
 * Helper class used for modifiable eval response items.
 * Original eval_reponse is non-modifiable which is rather unpractical in assignment.
 */
class eval_response_holder
{
private:
	/** ID of the job */
	std::string job_id;
	/** Textual representation of the evalution result */
	std::string result;
	/** Description of error if needed */
	std::string message;

public:
	/**
	 * Classical initialization constructor.
	 * @param job_id identification of job
	 * @param result textual representation of result
	 * @param message if execution failed then this should contain error message
	 */
	eval_response_holder(const std::string &job_id, const std::string &result, const std::string &message = "")
		: job_id(job_id), result(result), message(message)
	{
	}

	/**
	 * Set results to eval_response.
	 * @param res result aka status
	 * @param msg if execution failed then this should contain error message
	 */
	void set_result(const std::string &res, const std::string &msg = "")
	{
		result = res;
		message = msg;
	}

	/**
	 * Constructs and return eval_response with all possible information.
	 * @return eval_response structure
	 */
	eval_response get_eval_response()
	{
		return eval_response(job_id, result, message);
	}
};

#endif // RECODEX_WORKER_EVAL_RESPONSE_H

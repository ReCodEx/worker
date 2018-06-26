#ifndef RECODEX_WORKER_JOB_EVALUATOR_BASE_H
#define RECODEX_WORKER_JOB_EVALUATOR_BASE_H

#include "../eval_request.h"
#include "../eval_response.h"


/**
 * Interface of job evaluator.
 */
class job_evaluator_interface
{
public:
	/**
	 * Virtual destructor for proper destruction of inherited classes.
	 */
	virtual ~job_evaluator_interface() = default;

	/**
	 * Process an "eval" request
	 */
	virtual eval_response evaluate(eval_request request) = 0;
};

#endif // RECODEX_WORKER_JOB_EVALUATOR_BASE_H

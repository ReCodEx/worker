#ifndef CODEX_WORKER_JOB_EVALUATOR_BASE_H
#define CODEX_WORKER_JOB_EVALUATOR_BASE_H

#include "../eval_request.h"
#include "../eval_response.h"


/**
 * Base type of job evaluator.
 */
class job_evaluator_base
{
public:
	/**
	 * Virtual destructor for proper destruction of inherited classes.
	 */
	virtual ~job_evaluator_base()
	{
	}

	/**
	 * Process an "eval" request
	 */
	virtual eval_response evaluate(eval_request request) = 0;
};

#endif // CODEX_WORKER_JOB_EVALUATOR_BASE_H

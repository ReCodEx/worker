#ifndef CODEX_WORKER_JOB_REQUEST_H
#define CODEX_WORKER_JOB_REQUEST_H

#include <string>

/**
 * A structure that contains the information received with a job request
 */
struct eval_request {
	/** Id of given job, should be the same as the one stated in job config */
	const std::string job_id;
	/** Remote address where archive with job config and source code is located */
	const std::string job_url;
	/** Remote address on which results will be pushed */
	const std::string result_url;

	/**
	 * Construction of this structure with all variables set during it.
	 * @param job_id
	 * @param job_url
	 * @param result_url
	 */
	eval_request(std::string job_id, std::string job_url, std::string result_url)
		: job_id(job_id), job_url(job_url), result_url(result_url)
	{
	}
};

#endif // CODEX_WORKER_JOB_REQUEST_H

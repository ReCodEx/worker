#ifndef CODEX_WORKER_JOB_REQUEST_H
#define CODEX_WORKER_JOB_REQUEST_H

#include <string>

/**
 * A structure that contains the information received with a job request
 */
struct eval_request
{
	const std::string job_id;
	const std::string job_url;
	const std::string result_url;

	eval_request (std::string job_id, std::string job_url, std::string result_url) :
		job_id(job_id), job_url(job_url), result_url(result_url)
	{
	}
};

#endif //CODEX_WORKER_JOB_REQUEST_H

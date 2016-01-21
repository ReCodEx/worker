#ifndef CODEX_WORKER_JOB_CONFIG_H
#define CODEX_WORKER_JOB_CONFIG_H

#include <yaml-cpp/node/node.h>
#include "task_base.h"
#include "../config/worker_config.h"

/**
 * Configuration of a job assigned to the worker
 */
class job_metadata
{
public:
	job_metadata() = delete;

	/**
	 * Load the configuration from a YAML document
	 */
	job_metadata(const YAML::Node &config, std::shared_ptr<const worker_config> worker_config);

	/**
	 * Get the job identifier
	 */
	std::string get_job_id();

	/**
	 * Get the language of the submission
	 */
	std::string get_language();

	/**
	 * Get the URL of the file server where supplementary files are stored
	 */
	std::string get_file_server_url();

	/**
	 * Get the tasks to be processed (ordered by occurence in the configuration file)
	 */
	const std::vector<std::shared_ptr<task_base>> get_tasks();

private:
	std::string job_id_;
	std::string language_;
	std::string file_server_url_;
	std::vector<std::shared_ptr<task_base>> tasks_;
};

#endif //CODEX_WORKER_JOB_CONFIG_H

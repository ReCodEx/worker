#ifndef CODEX_WORKER_JOB_TASKS_H
#define CODEX_WORKER_JOB_TASKS_H

#include <yaml-cpp/yaml.h>
#include "job_metadata.h"
#include "../fileman/file_manager_base.h"

/**
 * A list of tasks to be processed
 */
class job_tasks
{
private:
	std::vector<std::shared_ptr<task_base>> tasks_;
	std::shared_ptr<worker_config> worker_config_;
public:
	job_tasks(
		const YAML::Node &config,
		std::shared_ptr<worker_config> worker_config,
		std::shared_ptr<file_manager_base> fileman
	);

	std::vector<std::shared_ptr<task_base>> get_tasks();

};


#endif //CODEX_WORKER_JOB_TASKS_H

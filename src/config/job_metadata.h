#ifndef CODEX_WORKER_JOB_CONFIG_H
#define CODEX_WORKER_JOB_CONFIG_H

#include <yaml-cpp/node/node.h>
#include "task_metadata.h"


/**
 *
 */
class job_metadata
{
public:
	std::string job_id = "";
	std::string language = "";
	std::string file_server_url = "";

	std::vector<std::shared_ptr<task_metadata>> tasks;
};

#endif //CODEX_WORKER_JOB_CONFIG_H

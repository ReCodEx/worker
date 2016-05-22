#ifndef CODEX_WORKER_JOB_CONFIG_H
#define CODEX_WORKER_JOB_CONFIG_H

#include <yaml-cpp/node/node.h>
#include "task_metadata.h"


/**
 * Overall information about job loaded from job configuration file.
 */
class job_metadata
{
public:
	/** Textual identificator of job. */
	std::string job_id = "";
	/** Textual description of language in which is solution written. Not needed, for debugging purposes. */
	std::string language = "";
	/** Address of file server from which datas can be downloaded through fetch task. */
	std::string file_server_url = "";
	/** Determines whether job log will be created or not. */
	bool log = false;

	/** List of tasks in same order as in configuration file. */
	std::vector<std::shared_ptr<task_metadata>> tasks;
};

#endif // CODEX_WORKER_JOB_CONFIG_H

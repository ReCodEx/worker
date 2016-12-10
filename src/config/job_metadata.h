#ifndef RECODEX_WORKER_JOB_CONFIG_H
#define RECODEX_WORKER_JOB_CONFIG_H

#include <yaml-cpp/node/node.h>
#include "task_metadata.h"


/**
 * Overall information about job loaded from job configuration file.
 * Whole job should be described here with all datas from configuration.
 */
class job_metadata
{
public:
	/** Textual identificator of job. */
	std::string job_id = "";
	/** Address of file server from which datas can be downloaded through fetch task. */
	std::string file_server_url = "";
	/** Determines whether job log will be created or not. */
	bool log = false;
	/** List of hwgroup for which this job can be applied. */
	std::vector<std::string> hwgroups;

	/** List of tasks in same order as in configuration file. */
	std::vector<std::shared_ptr<task_metadata>> tasks;
};

#endif // RECODEX_WORKER_JOB_CONFIG_H

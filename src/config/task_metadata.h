#ifndef CODEX_WORKER_TASK_METADATA_H
#define CODEX_WORKER_TASK_METADATA_H

#include "sandbox_limits.h"
#include "sandbox_config.h"


/**
 *
 */
class task_metadata
{
public:
	std::string task_id = "";
	size_t priority = 0;
	bool fatal_failure = false;
	std::vector<std::string> dependencies;

	std::string binary = "";
	std::vector<std::string> cmd_args;

	std::string std_input = "";
	std::string std_output = "";
	std::string std_error = "";

	std::shared_ptr<sandbox_config> sandbox = nullptr;
};


#endif //CODEX_WORKER_TASK_METADATA_H

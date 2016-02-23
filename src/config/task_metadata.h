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
	task_metadata(std::string task_id = "",
		size_t priority = 0,
		bool fatal = false,
		std::vector<std::string> deps = {},
		std::string cmd = "",
		std::vector<std::string> args = {},
		std::string input = "",
		std::string output = "",
		std::string err = "",
		std::shared_ptr<sandbox_config> sandbox = nullptr)
		: task_id(task_id), priority(priority), fatal_failure(fatal), dependencies(deps), binary(cmd), cmd_args(args),
		  std_input(input), std_output(output), std_error(err), sandbox(sandbox)
	{
	}

	std::string task_id;
	size_t priority;
	bool fatal_failure;
	std::vector<std::string> dependencies;

	std::string binary;
	std::vector<std::string> cmd_args;

	std::string std_input;
	std::string std_output;
	std::string std_error;

	std::shared_ptr<sandbox_config> sandbox;
};


#endif // CODEX_WORKER_TASK_METADATA_H

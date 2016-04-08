#ifndef CODEX_WORKER_TASK_METADATA_H
#define CODEX_WORKER_TASK_METADATA_H

#include "sandbox_limits.h"
#include "sandbox_config.h"


/**
 * Information about one task loaded from job configuration file.
 */
class task_metadata
{
public:
	/**
	 * Just constructor which takes all internal variables. Stated for better construction of class.
	 */
	task_metadata(std::string task_id = "",
		size_t priority = 0,
		bool fatal = false,
		std::vector<std::string> deps = {},
		std::string cmd = "",
		std::vector<std::string> args = {},
		std::shared_ptr<sandbox_config> sandbox = nullptr)
		: task_id(task_id), priority(priority), fatal_failure(fatal), dependencies(deps), binary(cmd), cmd_args(args),
		  sandbox(sandbox)
	{
	}

	/** Unique identifier of task in job. */
	std::string task_id;
	/** Priority of task among all others. Bigger priority number == greater priority. */
	size_t priority;
	/** If true than failure of task will end execution of whole job. */
	bool fatal_failure;
	/** Dependent tasks which have to be executed before this one. */
	std::vector<std::string> dependencies;

	/** Command which will be executed within this task. */
	std::string binary;
	/** Arguments for executed command. */
	std::vector<std::string> cmd_args;

	/** If not null than this task is external and will be executed in given sandbox. */
	std::shared_ptr<sandbox_config> sandbox;
};


#endif // CODEX_WORKER_TASK_METADATA_H

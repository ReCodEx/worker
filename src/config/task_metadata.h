#ifndef RECODEX_WORKER_TASK_METADATA_H
#define RECODEX_WORKER_TASK_METADATA_H

#include "sandbox_limits.h"
#include "sandbox_config.h"


/**
 * Type of task which can be present in configuration.
 */
enum class task_type { INNER, INITIATION, EXECUTION, EVALUATION };

/**
 * Information about one task loaded from job configuration file.
 */
class task_metadata
{
public:
	/**
	 * Just constructor which takes all internal variables. Stated for better construction of class.
	 * @param task_id identification of task in job configuration, default = ""
	 * @param priority priority of execution of task, default = 0
	 * @param fatal if task is fatal, then whole job ends of failure, default = false
	 * @param deps dependencies of this task, default = none
	 * @param type type of this task, default = INNER
	 * @param cmd command which will be executed, default = ""
	 * @param args arguments supplied for command, default = none
	 * @param sandbox configuration of sandbox, shared pointer, its data can be changed! default = nullptr
	 */
	task_metadata(const std::string &task_id = "",
		std::size_t priority = 0,
		bool fatal = false,
		std::vector<std::string> deps = {},
		task_type type = task_type::INNER,
		const std::string &cmd = "",
		std::vector<std::string> args = {},
		std::shared_ptr<sandbox_config> sandbox = nullptr,
		std::string test_id = "")
		: task_id(task_id), priority(priority), dependencies(deps), test_id(test_id), type(type), fatal_failure(fatal),
		  binary(cmd), cmd_args(args), sandbox(sandbox)
	{
	}

	/** Unique identifier of task in job. */
	std::string task_id;
	/** Priority of task among all others. Bigger priority number == greater priority. */
	std::size_t priority;
	/** Dependent tasks which have to be executed before this one. */
	std::vector<std::string> dependencies;
	/** Test id for external tasks */
	std::string test_id;

	/** Type of this task. */
	task_type type;
	/** If true than failure of task will end execution of whole job. */
	bool fatal_failure;

	/** Command which will be executed within this task. */
	std::string binary;
	/** Arguments for executed command. */
	std::vector<std::string> cmd_args;

	/** If not null than this task is external and will be executed in given sandbox. */
	std::shared_ptr<sandbox_config> sandbox;
};


#endif // RECODEX_WORKER_TASK_METADATA_H

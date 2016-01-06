#ifndef CODEX_WORKER_SANDBOX_LIMITS_H
#define CODEX_WORKER_SANDBOX_LIMITS_H

#include <map>


/**
* Sandbox limits.
* @note Not all sandbox implementations must follow all these parameters.
*/
struct sandbox_limits {
public:
	/**
	* Limit memory usage. For Isolate, this limits whole control group (--cg-mem switch).
	* Memory size is set in kilobytes.
	*/
	size_t memory_usage;
	/**
	* Limit total run time by CPU time. For Isolate, this is for whole control group.
	* Time is set in seconds and can be fractional.
	*/
	float cpu_time;
	/**
	* Limit total run time by wall clock. Time is set in seconds and can be fractional.
	*/
	float wall_time;
	/**
	* Set extra time before kill the process. If program finishes in this extra amount of
	* time, it won't succeeded, but total run time will be reported to results log. This
	* time is also in (fractional) seconds.
	*/
	float extra_time;
	/**
	* Limit stack size. This is additional memory limit, 0 is no special limit for stack,
	* global memory rules will aply. Otherwise, max stack size is @a stack_size kilobytes.
	*/
	size_t stack_size;
	/**
	* Limit size of created files. This could be useful, if your filesystem doesn't support
	* quotas. Otherwise, see items @a disk_blocks and @a disk_inodes. 0 means not set.
	*/
	size_t files_size;
	/**
	* Set disk quota to given number of blocks.
	* @warning Underlying filesystem must support quotas.
	*/
	size_t disk_blocks;
	/**
	* Set disk quota to given number of inodes.
	* @warning Underlying filesystem must support quotas.
	*/
	size_t disk_inodes;
	/**
	* Redirect standard input from given file.
	* @note Path must be accessible from inside of sandbox.
	*/
	std::string std_input;
	/**
	* Redirect standard output to given file.
	* @note Path must be accessible from inside of sandbox.
	*/
	std::string std_output;
	/**
	* Redirect standard error output to given file.
	* @note Path must be accessible from inside of sandbox.
	*/
	std::string std_error;
	/**
	* Change working directory to subdirectory inside the sandbox.
	* @note Path must be accessible from inside of sandbox.
	*/
	std::string chdir;
	/**
	* Limit number of processes/threads that could be created.
	* 0 means no limit.
	*/
	size_t processes;
	/**
	* Allow to share host computers network. Otherwise, dedicated
	* local interface will be created.
	*/
	bool share_net;
	/**
	* Set environment variables before run command inside the sandbox.
	*/
	std::map<std::string, std::string> environ_vars;


	bool operator==(const sandbox_limits &second) const
	{
		if (memory_usage != second.memory_usage ||
			cpu_time != second.cpu_time ||
			wall_time != second.wall_time ||
			extra_time != second.extra_time ||
			stack_size != second.stack_size ||
			files_size != second.files_size ||
			disk_blocks != second.disk_blocks ||
			disk_inodes != second.disk_inodes ||
			std_input != second.std_input ||
			std_output != second.std_output ||
			std_error != second.std_error ||
			chdir != second.chdir ||
			processes != second.processes ||
			share_net != second.share_net ||
			environ_vars != second.environ_vars) {
			return false;
		}

		return true;
	}

	bool operator!=(const sandbox_limits &second) const
	{
		return !((*this) == second);
	}
};

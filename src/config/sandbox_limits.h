#ifndef CODEX_WORKER_SANDBOX_LIMITS_H
#define CODEX_WORKER_SANDBOX_LIMITS_H

#include <map>


/**
 *
 */
struct sandbox_limits {
public:
	size_t memory_usage = 0;	//limit memory usage by whole control group (kB)
	float cpu_time = 0;			//limit total run time of whole control group (s)
	float wall_time = 0;		//limit wall time of the program (s)
	float extra_time = 0;		//wait extra time after time limit exceeded (s)
	size_t stack_size = 0;		//limit stack size (kB), 0 = no limit
	size_t files_size = 0;		//limit disk usage (kB), 0 = no limit
	size_t disk_blocks = 0;		//disk quota in blocks
	size_t disk_inodes = 0;		//disk quota in inodes
	std::string std_input;		//redirect stdin from file
	std::string std_output;		//redirect stdout to file
	std::string std_error;		//redirect stderr to file
	std::string chdir;		//change working directory inside sandbox
	size_t processes = 0;		//limit number of processes, 0 = no limit
	bool share_net = false;			//allow use host network
	std::map<std::string, std::string> environ_vars; //set environment variables

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

#endif //CODEX_WORKER_SANDBOX_LIMITS_H

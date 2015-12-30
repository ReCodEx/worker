#ifndef CODEX_WORKER_FILE_SANDBOX_BASE_H
#define CODEX_WORKER_FILE_SANDBOX_BASE_H

#include <memory>
#include <string>
#include <exception>
#include <map>
#include <vector>

/**
 * Sandbox limits.
 * @note Not all sandbox implementations must follow all these parameters.
 */
struct sandbox_limits {
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
	std::string stdin;
	/**
	 * Redirect standard output to given file.
	 * @note Path must be accessible from inside of sandbox.
	 */
	std::string stdout;
	/**
	 * Redirect standard error output to given file.
	 * @note Path must be accessible from inside of sandbox.
	 */
	std::string stderr;
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
	std::map<std::string, std::string> environ_vars; //set environment variables
};

/**
 * Return error codes of sandbox.
 */
enum class isolate_status { RE, SG, TO, XX, NOTSET };

/**
 * Sandbox results.
 * @note Not all items must be returned from sandbox, so some defaults may aply.
 */
struct task_results {
	/**
	 * Return code of sandbox.
	 * Defaul: 0
	 */
	int exitcode;
	/**
	 * Total run time of program inside the sandbox.
	 * Default: 0 (s)
	 */
	float time;
	/**
	 * Total run time (wall clock) of program inside the sandbox.
	 * Default: 0 (s)
	 */
	float wall_time;
	/**
	 * Amount of memory used by program inside the sandbox.
	 * Default: 0 (kB)
	 */
	size_t memory;
	/**
	 * Maximum resident set size of the process.
	 * Default: 0 (kB)
	 */
	size_t max_rss;
	/**
	 * Error code returned by sandbox.
	 * Default: NOTSET
	 */
	isolate_status status;
	/**
	 * Signal, which killed the process.
	 * Default: 0
	 */
	int exitsig;
	/**
	 * Flag if program exited normaly or was killed.
	 * Default: false
	 */
	bool killed;
	/**
	 * Error message of the sandbox.
	 * Default: ""
	 */
	std::string message;
};


/**
 * Base class for all sandbox implementations.
 */
class sandbox_base {
public:
	/**
	 * Destructor.
	 */
	virtual ~sandbox_base() {}
	/**
	 * Get sandboxed directory (to copy files inside, ...)
	 */
	virtual std::string get_dir() const { return sandboxed_dir_; }
	/**
	 * Run sandbox.
	 * @param binary Name of binary to run. Must be accessible from inside the sandbox.
	 * @param arguments Commandline arguments to the binary.
	 * @return Sandbox results.
	 */
	virtual task_results run(const std::string &binary, const std::vector<std::string> &arguments) = 0;
protected:
	std::string sandboxed_dir_;
};


/**
 * Common exception for all sandbox implementations.
 */
class sandbox_exception : public std::exception {
public:
	/**
	 * Default constructor.
	 */
	sandbox_exception() : what_("Generic sandbox exception") {}
	/**
	 * Constructor with custom message.
	 * @param what Custom message.
	 */
	sandbox_exception(const std::string &what) : what_(what) {}
	/**
	 * Destructor.
	 */
	virtual ~sandbox_exception() {}
	/**
	 * Get message.
	 */
	virtual const char* what() const noexcept
	{
		return what_.c_str();
	}
protected:
	std::string what_;
};

#endif // CODEX_WORKER_FILE_SANDBOX_BASE_H

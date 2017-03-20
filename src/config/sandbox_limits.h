#ifndef RECODEX_WORKER_SANDBOX_LIMITS_H
#define RECODEX_WORKER_SANDBOX_LIMITS_H

#include <cfloat>
#include <limits>
#include <vector>
#include <tuple>
#include <utility>


/**
* Sandbox limits which are sent to sandbox during execution of given task.
* @note Not all sandbox implementations must follow all these parameters.
*/
struct sandbox_limits {
public:
	/**
	 * Allowed permissions for directory bindings.
	 * @param RO Read only.
	 * @param RW Read and write access.
	 * @param NOEXEC Disallow execution of binaries.
	 * @param FS Instead of binding a directory, mount a device-less filesystem.
	 * @param MAYBE Silently ignore the rule if the directory to be bound does not exist.
	 * @param DEV Allow access to character and block devices.
	 * @warning Not all options must be supported by all sandboxes. Please, consult your sandbox documentation first.
	 */
	enum dir_perm : unsigned short { RO = 0, RW = 1, NOEXEC = 2, FS = 4, MAYBE = 8, DEV = 16 };
	/**
	 * Limit memory usage. For Isolate, this limits whole control group (--cg-mem switch).
	 * Memory size is set in kilobytes.
	 */
	size_t memory_usage = 0;
	/**
	 * Limit total run time by CPU time. For Isolate, this is for whole control group.
	 * Time is set in seconds and can be fractional.
	 */
	float cpu_time = 0;
	/**
	 * Limit total run time by wall clock. Time is set in seconds and can be fractional.
	 */
	float wall_time = 0;
	/**
	 * Set extra time before kill the process. If program finishes in this extra amount of
	 * time, it won't succeeded, but total run time will be reported to results log. This
	 * time is also in (fractional) seconds.
	 */
	float extra_time = 0;
	/**
	 * Limit stack size. This is additional memory limit, 0 is no special limit for stack,
	 * global memory rules will aply. Otherwise, max stack size is @a stack_size kilobytes.
	 */
	size_t stack_size = 0;
	/**
	 * Limit size of created files. This could be useful, if your filesystem doesn't support
	 * quotas. 0 means not set.
	 * @warning This option is deprecated! Use @ref disk_size and @ref disk_files instead.
	 */
	size_t files_size = 0;
	/**
	 * Set disk quota to given number of kilobytes.
	 * @warning Underlying filesystem must support quotas.
	 */
	size_t disk_size = 0;
	/**
	 * Set disk quota to given number of files. Actual implementation may vary, for example
	 * on Linux with ext4 filesystem this should be maximum number of used inodes.
	 * @warning Underlying filesystem must support quotas.
	 */
	size_t disk_files = 0;
	/**
	 * Change working directory to subdirectory inside the sandbox.
	 * @note Path must be accessible from inside of sandbox.
	 */
	std::string chdir;
	/**
	 * Limit number of processes/threads that could be created.
	 * 0 means no limit.
	 */
	size_t processes = 0;
	/**
	 * Allow to share host computers network. Otherwise, dedicated
	 * local interface will be created.
	 */
	bool share_net = false;
	/**
	 * Set environment variables before run command inside the sandbox.
	 */
	std::vector<std::pair<std::string, std::string>> environ_vars;
	/**
	 * Contains local directories that should be bound into the sandbox.
	 */
	std::vector<std::tuple<std::string, std::string, dir_perm>> bound_dirs;


	/**
	 * Constructor with some defaults.
	 */
	sandbox_limits()
	{
		chdir = "${EVAL_DIR}";
		bound_dirs.push_back(std::tuple<std::string, std::string, sandbox_limits::dir_perm>(
			"${SOURCE_DIR}", "${EVAL_DIR}", dir_perm::RW));
	}

	/**
	 * Classical equality operator on two structures.
	 * @param second compared structure
	 * @return true if compared structures has same variable values
	 */
	bool operator==(const sandbox_limits &second) const
	{
		return (memory_usage == second.memory_usage && cpu_time == second.cpu_time && wall_time == second.wall_time &&
			extra_time == second.extra_time && stack_size == second.stack_size && files_size == second.files_size &&
			disk_size == second.disk_size && disk_files == second.disk_files && chdir == second.chdir &&
			processes == second.processes && share_net == second.share_net && environ_vars == second.environ_vars &&
			bound_dirs == second.bound_dirs);
	}

	/**
	 * Opposite of equality operator.
	 * @param second compared structure
	 * @return true if structures are not the same
	 */
	bool operator!=(const sandbox_limits &second) const
	{
		return !((*this) == second);
	}
};

#endif // RECODEX_WORKER_SANDBOX_LIMITS_H

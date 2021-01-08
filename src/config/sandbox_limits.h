#ifndef RECODEX_WORKER_SANDBOX_LIMITS_H
#define RECODEX_WORKER_SANDBOX_LIMITS_H

#include <cfloat>
#include <limits>
#include <vector>
#include <tuple>
#include <utility>
#include <algorithm>
#include <map>
#include <string>
#include "helpers/type_utils.h"


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
	enum dir_perm : unsigned short { RO = 0, RW = 1, NOEXEC = 2, FS = 4, MAYBE = 8, DEV = 16, TMP = 32, NOREC = 64 };
	/**
	 * Return a mapping between dir_perm enums and their associated string representatives.
	 */
	static const std::map<sandbox_limits::dir_perm, std::string> &get_dir_perm_associated_strings()
	{
		static std::map<sandbox_limits::dir_perm, std::string> options;
		if (options.size() == 0) {
			// fill the map for the first time
			options[sandbox_limits::dir_perm::RW] = "rw";
			options[sandbox_limits::dir_perm::NOEXEC] = "noexec";
			options[sandbox_limits::dir_perm::FS] = "fs";
			options[sandbox_limits::dir_perm::MAYBE] = "maybe";
			options[sandbox_limits::dir_perm::DEV] = "dev";
			options[sandbox_limits::dir_perm::TMP] = "tmp";
			options[sandbox_limits::dir_perm::NOREC] = "norec";
		}
		return options;
	}
	/**
	 * Limit memory usage. For Isolate, this limits whole control group (--cg-mem switch).
	 * Memory size is set in kilobytes.
	 */
	std::size_t memory_usage = 0;
	/**
	 * Extra memory which will be added to memory limit before killing program.
	 * Memory size is set in kilobytes.
	 */
	std::size_t extra_memory = 0;
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
	 * Allow to share host computers network. Otherwise, dedicated
	 * local interface will be created.
	 */
	bool share_net = false;
	/**
	 * Limit stack size. This is additional memory limit, 0 is no special limit for stack,
	 * global memory rules will aply. Otherwise, max stack size is @a stack_size kilobytes.
	 */
	std::size_t stack_size = 0;
	/**
	 * Limit size of created files. This could be useful, if your filesystem doesn't support
	 * quotas. 0 means not set.
	 * @warning This option is deprecated! Use @ref disk_size and @ref disk_files instead.
	 */
	std::size_t files_size = 0;
	/**
	 * Whether disk quotas (disk_size and disk_files) are enabled.
	 * @warning Keep this false if underlying filesystem does not support quotas.
	 */
	bool disk_quotas = false;
	/**
	 * Set disk quota to given number of kilobytes.
	 * @warning Underlying filesystem must support quotas.
	 */
	std::size_t disk_size = 0;
	/**
	 * Set disk quota to given number of files. Actual implementation may vary, for example
	 * on Linux with ext4 filesystem this should be maximum number of used inodes.
	 * @warning Underlying filesystem must support quotas.
	 */
	std::size_t disk_files = 0;
	/**
	 * Limit number of processes/threads that could be created.
	 * 0 means no limit.
	 */
	std::size_t processes = 0;
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
	sandbox_limits() = default;

	/**
	 * Insert environment variables which are not present yet.
	 * @brief add_environ_vars
	 * @param vars variables which will be added
	 */
	void add_environ_vars(std::vector<std::pair<std::string, std::string>> &vars)
	{
		for (auto &var : vars) {
			if (std::find(environ_vars.begin(), environ_vars.end(), var) != environ_vars.end()) { continue; }
			environ_vars.push_back(var);
		}
	}

	/**
	  Insert bound directories which are not present yet.
	 * @brief add_bound_dirs
	 * @param dirs directories which will be added
	 */
	void add_bound_dirs(std::vector<std::tuple<std::string, std::string, dir_perm>> &dirs)
	{
		for (auto &dir : dirs) {
			if (std::find(bound_dirs.begin(), bound_dirs.end(), dir) != bound_dirs.end()) { continue; }
			bound_dirs.push_back(dir);
		}
	}

	/**
	 * Classical equality operator on two structures.
	 * @param second compared structure
	 * @return true if compared structures has same variable values
	 */
	bool operator==(const sandbox_limits &second) const
	{
		return (memory_usage == second.memory_usage && extra_memory == second.extra_memory &&
			helpers::almost_equal(cpu_time, second.cpu_time) && helpers::almost_equal(wall_time, second.wall_time) &&
			helpers::almost_equal(extra_time, second.extra_time) && stack_size == second.stack_size &&
			files_size == second.files_size && disk_size == second.disk_size && disk_files == second.disk_files &&
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

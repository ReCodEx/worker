#ifndef RECODEX_WORKER_FILE_ISOLATE_SANDBOX_H
#define RECODEX_WORKER_FILE_ISOLATE_SANDBOX_H

#ifndef _WIN32

#include <memory>
#include <vector>
#include "sandbox_base.h"

/**
 * Class implementing operations with Isolate sandbox.
 *
 * Sandbox is used for security of system running untrusted program. Isolate itself
 * sets restrictions to the application like time limit, memory limit or accessible
 * files. When any of the limits are reached, the program inside sandbox is killed.
 * To improve reliability and safety, isolate is run in separate process and there
 * is also external time limit for that process to run. This limit is counted as
 * maximum execution time of program inside sandbox plus 300 seconds (5 minutes)
 * to have some base time even for very short tasks. That sum is multiplied by factor
 * of 1.2 which gives total maximum time of running isolate. After that time, isolate's
 * thread is killed. Note that this time limit should not be restrictive in normal
 * usage, but it's another safety feature when the app inside can break isolate (which
 * is unlikely).
 *
 * @note Requirements are Linux OS with Isolate installed. For detailed instructions see
 * Isolate's manual page. Isolate binary must be named "isolate" and must be in PATH
 * (default installer of Isolate meets these requirements).
 */
class isolate_sandbox : public sandbox_base
{
public:
	/**
	 * Constructor.
	 * @param limits Limits for current command.
	 * @param id Number of current worker. This must be unique for each worker on one machine!
	 * @param temp_dir Directory to store temporary files (generated isolate's meta log)
	 * @param data_dit Directory containing sources which will be copied into sandbox
	 * @param logger Set system logger (optional).
	 */
	isolate_sandbox(std::shared_ptr<sandbox_config> sandbox_config,
		sandbox_limits limits,
		std::size_t id,
		const std::string &temp_dir,
		const std::string &data_dir,
		std::shared_ptr<spdlog::logger> logger = nullptr);

private:
	/** Path and name of isolate's meta file - here are stored informations about evaluation */
	std::string meta_file_;

	/** Initialize isolate (called in the constructor) */
	void sandbox_init() override;

	/** Run isolate evaluation with sandboxed program inside. */
	void sandbox_run(const std::string &binary, const std::vector<std::string> &arguments) override;

	/** Cleanup isolate after finish evaluation (called in the destructor) */
	void sandbox_cleanup() override;

	/** Actual code for isolate initialization inside a process. Called by sandbox_init(). */
	void isolate_init_child();

	/** Get isolate command line arguments as plain C string including sandboxed binary with its arguments. */
	char **isolate_run_args(const std::string &binary, const std::vector<std::string> &arguments);

	/** Parse isolate's meta file with evaluation informations. Must be called after sandbox_run() method. */
	sandbox_results extract_results() override;
};


#endif // _WIN32
#endif // RECODEX_WORKER_FILE_ISOLATE_SANDBOX_H

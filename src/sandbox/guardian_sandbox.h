#ifndef RECODEX_WORKER_FILE_GUARDIAN_SANDBOX_H
#define RECODEX_WORKER_FILE_GUARDIAN_SANDBOX_H

#ifndef _WIN32

#include <memory>
#include <vector>
#include "helpers/logger.h"
#include "sandbox_base.h"
#include "config/sandbox_config.h"

/**
 * Class implementing operations with ReCodEx Guardian sandbox.
 *
 * Right now, guardian mimics the CLI API of isolate, so it can be used as
 * direct replacement. This will be gradually modified in the future.
 */
class guardian_sandbox : public sandbox_base
{
public:
	/**
	 * Constructor.
	 * @param limits Limits for current command.
	 * @param id Number of current worker. This must be unique for each worker on one machine!
	 * @param temp_dir Directory to store temporary files (generated sandbox's meta log)
	 * @param data_dit Directory containing sources which will be copied into sandbox
	 * @param logger Set system logger (optional).
	 */
	guardian_sandbox(std::shared_ptr<sandbox_config> sandbox_config,
		sandbox_limits limits,
		std::size_t id,
		const std::string &temp_dir,
		const std::string &data_dir,
		std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Destructor.
	 */
	~guardian_sandbox() override;
	sandbox_results run(const std::string &binary, const std::vector<std::string> &arguments) override;

private:
	/** General sandbox configuration */
	std::shared_ptr<sandbox_config> sandbox_config_;
	/** Limits for sandboxed program */
	sandbox_limits limits_;
	/** Logger */
	std::shared_ptr<spdlog::logger> logger_;
	/** Identifier of this guardian's instance. Must be unique on each server. */
	std::size_t id_;
	/** Name of guardian binary - defaults "recodex-guardian" */
	std::string guardian_binary_;
	/** Path to temporary directory used by sandboxes. Subdir with "id_" value will be created. */
	std::string temp_dir_;
	/** Path and name of guardian's meta file - here are stored informations about evaluation */
	std::string meta_file_;
	/** Maximum time to run separate guardian process */
	int max_timeout_;
	/** Path to the directory containing sources moved to sandbox and back */
	std::string data_dir_;
	/** Initialize sandbox */
	void guardian_init();
	/** Actual code for guardian initialization inside a process. Called by guardian_init(). */
	void guardian_init_child(int fd_0, int fd_1);
	/** Cleanup guardian after finish evaluation */
	void guardian_cleanup();
	/** Run guardian evaluation with sandboxed program inside. */
	void guardian_run(const std::string &binary, const std::vector<std::string> &arguments);
	/** Get guardian command line arguments as plain C string including sandboxed binary with its arguments. */
	char **guardian_run_args(const std::string &binary, const std::vector<std::string> &arguments);
	/** Parse guardian's meta file with evaluation informations. Must be called after guardian_run() method. */
	sandbox_results process_meta_file();
};


#endif // _WIN32
#endif // RECODEX_WORKER_FILE_GUARDIAN_SANDBOX_H

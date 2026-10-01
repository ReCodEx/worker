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
	 * @param sandbox_config General sandbox configuration.
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

private:
	/** Path and name of guardian's meta file - here are stored informations about evaluation */
	std::string meta_file_;

	/** Initialize guardian (called in the constructor) */
	void sandbox_init() override;

	/** Run guardian evaluation with sandboxed program inside. */
	void sandbox_run(const std::string &binary, const std::vector<std::string> &arguments) override;

	/** Cleanup guardian after the evaluation (called by the destructor) */
	void sandbox_cleanup() override;

	/** Actual code for guardian initialization inside a process. Called by sandbox_init(). */
	void guardian_init_child();

	/** Get guardian command line arguments as plain C string including sandboxed binary with its arguments. */
	char **guardian_run_args(const std::string &binary, const std::vector<std::string> &arguments);

	/** Parse guardian's meta file with evaluation informations. Must be called after sandbox_run() method. */
	sandbox_results extract_results() override;
};


#endif // _WIN32
#endif // RECODEX_WORKER_FILE_GUARDIAN_SANDBOX_H

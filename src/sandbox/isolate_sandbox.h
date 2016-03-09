#ifndef CODEX_WORKER_FILE_ISOLATE_SANDBOX_H
#define CODEX_WORKER_FILE_ISOLATE_SANDBOX_H

#ifndef _WIN32

#include <memory>
#include <vector>
#include "../helpers/create_logger.h"
#include "sandbox_base.h"

/**
 * Class implementing operations with Isolate sandbox.
 * @note Requirements are Linux OS with Isolate installed. Isolate binary must be named "isolate"
 * and must be in PATH (default installer of Isolate meets these requirements).
 */
class isolate_sandbox : public sandbox_base
{
public:
	/**
	 * Constructor.
	 * @param limits Limits for current command.
	 * @param id Number of current worker. This must be unique for each worker on one machine!
	 * @param temp_dir Directory to store temporary files (generated isolate's meta log)
	 * @param logger Set system logger (optional).
	 */
	isolate_sandbox(sandbox_limits limits,
		size_t id,
		const std::string &temp_dir,
		std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Destructor.
	 */
	virtual ~isolate_sandbox();
	virtual sandbox_results run(const std::string &binary, const std::vector<std::string> &arguments);

private:
	sandbox_limits limits_;
	std::shared_ptr<spdlog::logger> logger_;
	size_t id_;
	std::string isolate_binary_;
	std::string temp_dir_;
	std::string meta_file_;
	int max_timeout_;
	void isolate_init();
	void isolate_init_child(int fd_0, int fd_1);
	void isolate_cleanup();
	void isolate_run(const std::string &binary, const std::vector<std::string> &arguments);
	char **isolate_run_args(const std::string &binary, const std::vector<std::string> &arguments);
	sandbox_results process_meta_file();
};


#endif // _WIN32
#endif // CODEX_WORKER_FILE_ISOLATE_SANDBOX_H

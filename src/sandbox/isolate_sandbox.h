#ifndef CODEX_WORKER_FILE_ISOLATE_SANDBOX_H
#define CODEX_WORKER_FILE_ISOLATE_SANDBOX_H

#ifndef _WIN32

#include <memory>
#include <vector>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "sandbox_base.h"

/**
 * Class implementing operations with Isolate sandbox.
 * @note Requirements are Linux OS with Isolate installed. Isolate binary must be named "isolate"
 * and must be in PATH.
 */
class isolate_sandbox : public sandbox_base {
public:
	/**
	 * Constructor.
	 * @param limits Limits for current command.
	 * @param id Number of current worker. This must be unique for each worker on one machine!
	 * @param max_timeout Second security level - sandbox must end in this limit othrwise will be killed.
	 * If not set (or set to -1), this number will be safely computed from @a limits. This limit is in seconds (optional).
	 * @param logger Set system logger (optional).
	 */
	isolate_sandbox(sandbox_limits limits, size_t id, int max_timeout = -1,
					std::shared_ptr<spdlog::logger> logger = nullptr);
	/**
	 * Destructor.
	 */
	virtual ~isolate_sandbox();
	virtual task_results run(const std::string &binary, const std::vector<std::string> &arguments);
private:
	sandbox_limits limits_;
	std::shared_ptr<spdlog::logger> logger_;
	size_t id_;
	std::string isolate_binary_;
	std::string meta_file_;
	int max_timeout_;
	void isolate_init();
	void isolate_cleanup();
	void isolate_run(const std::string &binary, const std::vector<std::string> &arguments);
	char **isolate_run_args(const std::string &binary, const std::vector<std::string> &arguments);
	task_results process_meta_file();
};


#endif // _WIN32
#endif // CODEX_WORKER_FILE_ISOLATE_SANDBOX_H

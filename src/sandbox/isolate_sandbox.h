#ifndef CODEX_WORKER_FILE_ISOLATE_SANDBOX_H
#define CODEX_WORKER_FILE_ISOLATE_SANDBOX_H

#ifndef _WIN32

#include <memory>
#include <vector>
#include "spdlog/spdlog.h"
#include "sandbox_base.h"


class isolate_sandbox : public sandbox_base {
public:
	isolate_sandbox(sandbox_limits limits, size_t id, int max_timeout = -1,
					std::shared_ptr<spdlog::logger> logger = nullptr);
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

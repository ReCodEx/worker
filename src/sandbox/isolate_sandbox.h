#ifndef CODEX_WORKER_FILE_ISOLATE_SANDBOX_H
#define CODEX_WORKER_FILE_ISOLATE_SANDBOX_H

#include <memory>
#include "spdlog/spdlog.h"
#include "sandbox_base.h"


class isolate_sandbox : public sandbox_base {
public:
	isolate_sandbox(sandbox_limits limits, size_t id, std::shared_ptr<spdlog::logger> logger = nullptr);
	virtual ~isolate_sandbox();
	virtual task_results run(const std::string &binary, const std::string &arguments);
private:
	sandbox_limits limits_;
	std::shared_ptr<spdlog::logger> logger_;
	size_t id_;
	std::string isolate_binary_;
	void isolate_init();
};


#endif // CODEX_WORKER_FILE_ISOLATE_SANDBOX_H

#ifndef CODEX_WORKER_EXTERNAL_TASK_HPP
#define CODEX_WORKER_EXTERNAL_TASK_HPP

#include "task_base.h"
#include "../sandbox/sandbox_base.h"
#include "../sandbox/isolate_sandbox.h"


/**
 *
 */
class external_task : public task_base {
public:
	external_task() = delete;

	external_task(const std::string &binary, const std::vector<std::string> &arguments,
				  const std::string &sandbox, sandbox_limits limits);
	virtual ~external_task();

	virtual void run();
private:

	void sandbox_init();

	std::string cmd_;
	std::vector<std::string> args_;
	std::string sandbox_id_;
	std::shared_ptr<sandbox_base> sandbox_;
	sandbox_limits limits_;
};

#endif //CODEX_WORKER_EXTERNAL_TASK_HPP

#ifndef CODEX_WORKER_FAKE_SANDBOX_H
#define CODEX_WORKER_FAKE_SANDBOX_H

#include "sandbox_base.h"


/**
 * Fake sandbox for testing purposes.
 */
class fake_sandbox : public sandbox_base
{
public:
	/**
	 * Constructor.
	 */
	fake_sandbox();
	/**
	 * Destructor.
	 */
	virtual ~fake_sandbox();
	/**
	 * Run method which do nothing just return empty task_results structure.
	 * @param binary
	 * @param arguments
	 * @return
	 */
	virtual sandbox_results run(const std::string &binary, const std::vector<std::string> &arguments);
};


#endif // CODEX_WORKER_FAKE_SANDBOX_H

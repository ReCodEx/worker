#ifndef CODEX_WORKER_FAKE_TASK_HPP
#define CODEX_WORKER_FAKE_TASK_HPP

#include <memory>
#include "task_base.h"


/**
 *
 */
class fake_task : public task_base {
public:
	fake_task();
	virtual ~fake_task();

	virtual void run();
};

#endif //CODEX_WORKER_FAKE_TASK_HPP

#ifndef CODEX_WORKER_TASK_BASE_HPP
#define CODEX_WORKER_TASK_BASE_HPP

#include <vector>


/**
 *
 */
class task_base {
public:
	virtual void run() = 0;
private:
	size_t task_id_;
	std::vector<task_base> parents_;
	std::vector<task_base> children_;
	size_t priority_;
};

#endif //CODEX_WORKER_TASK_BASE_HPP

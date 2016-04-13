#ifndef CODEX_WORKER_TASK_FACTORY_BASE_H
#define CODEX_WORKER_TASK_FACTORY_BASE_H

#include <memory>
#include "task_base.h"
#include "create_params.h"
#include "../fileman/file_manager_base.h"



class task_factory_base {
public:
	virtual ~task_factory_base() {}
	virtual std::shared_ptr<task_base> create_internal_task(size_t id,
		std::shared_ptr<task_metadata> task_meta = nullptr) = 0;
	virtual std::shared_ptr<task_base> create_sandboxed_task(const create_params &data) = 0;
};


#endif // CODEX_WORKER_TASK_FACTORY_BASE_H

#ifndef CODEX_WORKER_TASK_FACTORY_H
#define CODEX_WORKER_TASK_FACTORY_H

#include <memory>
#include "task_factory_base.h"
#include "external_task.h"
#include "root_task.h"
#include "internal/archivate_task.h"
#include "internal/cp_task.h"
#include "internal/extract_task.h"
#include "internal/fetch_task.h"
#include "internal/mkdir_task.h"
#include "internal/rename_task.h"
#include "internal/rm_task.h"
#include "../fileman/file_manager_base.h"


class task_factory : public task_factory_base
{
public:
	task_factory(std::shared_ptr<file_manager_base> fileman);

	virtual std::shared_ptr<task_base> create_internal_task(
		size_t id, std::shared_ptr<task_metadata> task_meta = nullptr);
	virtual std::shared_ptr<task_base> create_sandboxed_task(const create_params &data);

private:
	std::shared_ptr<file_manager_base> fileman_;
};


#endif // CODEX_WORKER_TASK_FACTORY_H

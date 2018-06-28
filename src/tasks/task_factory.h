#ifndef RECODEX_WORKER_TASK_FACTORY_H
#define RECODEX_WORKER_TASK_FACTORY_H

#include <memory>
#include "task_factory_interface.h"
#include "external_task.h"
#include "root_task.h"
#include "internal/archivate_task.h"
#include "internal/cp_task.h"
#include "internal/dump_dir_task.h"
#include "internal/truncate_task.h"
#include "internal/extract_task.h"
#include "internal/fetch_task.h"
#include "internal/mkdir_task.h"
#include "internal/rename_task.h"
#include "internal/rm_task.h"
#include "internal/exists_task.h"
#include "fileman/file_manager_interface.h"


/**
 * Main task factory used by @ref job class.
 */
class task_factory : public task_factory_interface
{
public:
	/**
	 * Constructor
	 * @param fileman Instance of file manager to be used. It's required by @ref fetch_task to work properly.
	 */
	task_factory(std::shared_ptr<file_manager_interface> fileman);

	/**
	 * Virtual destructor
	 */
	~task_factory() override = default;

	/**
	 * Create internal task. This could be one of predefined operations like move or copy file,
	 * extract archive etc. which don't need sandbox to execute or root task, ie. base holder
	 * for all tasks in dependency graph.
	 * @param id Identifier of newly created task.
	 * @param task_meta Metadata for newly created task. If this parameter is @a nullptr or is not
	 * specified, @ref root_task is created. Otherwise, type of created task is detected from given
	 * metadata.
	 * @return Pointer to task's base type holding proper task type. If requested task type is unknown, @a nullptr
	 * is returned.
	 */
	std::shared_ptr<task_base> create_internal_task(
		std::size_t id, std::shared_ptr<task_metadata> task_meta = nullptr) override;

	/**
	 * Created task which will run in sandboxed environment.
	 * @param data Structure holding creating parameters for external (sandboxed) tasks.
	 * @return Pointer to task's base type holding proper task type.
	 */
	std::shared_ptr<task_base> create_sandboxed_task(const create_params &data) override;

private:
	/** Pointer to given file manager instance. */
	std::shared_ptr<file_manager_interface> fileman_;
};


#endif // RECODEX_WORKER_TASK_FACTORY_H

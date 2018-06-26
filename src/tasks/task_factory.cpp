#include "task_factory.h"


task_factory::task_factory(std::shared_ptr<file_manager_interface> fileman) : fileman_(fileman)
{
}

std::shared_ptr<task_base> task_factory::create_internal_task(size_t id, std::shared_ptr<task_metadata> task_meta)
{
	std::shared_ptr<task_base> task;

	if (task_meta == nullptr) { return std::make_shared<root_task>(id); }

	if (task_meta->binary == "cp") {
		task = std::make_shared<cp_task>(id, task_meta);
	} else if (task_meta->binary == "dumpdir") {
		task = std::make_shared<dump_dir_task>(id, task_meta);
	} else if (task_meta->binary == "mkdir") {
		task = std::make_shared<mkdir_task>(id, task_meta);
	} else if (task_meta->binary == "rename") {
		task = std::make_shared<rename_task>(id, task_meta);
	} else if (task_meta->binary == "rm") {
		task = std::make_shared<rm_task>(id, task_meta);
	} else if (task_meta->binary == "archivate") {
		task = std::make_shared<archivate_task>(id, task_meta);
	} else if (task_meta->binary == "extract") {
		task = std::make_shared<extract_task>(id, task_meta);
	} else if (task_meta->binary == "fetch") {
		task = std::make_shared<fetch_task>(id, task_meta, fileman_);
	} else if (task_meta->binary == "truncate") {
		task = std::make_shared<truncate_task>(id, task_meta);
	} else if (task_meta->binary == "exists") {
		task = std::make_shared<exists_task>(id, task_meta);
	} else {
		task = nullptr;
	}

	return task;
}

std::shared_ptr<task_base> task_factory::create_sandboxed_task(const create_params &data)
{
	return std::make_shared<external_task>(data);
}

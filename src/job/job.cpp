#include "job.h"
#include "job_exception.h"

job::job(std::shared_ptr<job_metadata> job_meta, std::shared_ptr<worker_config> worker_conf,
		 fs::path source_path, std::shared_ptr<file_manager_base> fileman)
	: job_meta_(job_meta), worker_config_(worker_conf), source_path_(source_path), fileman_(fileman)
{
	// check construction parameters if they are in right format
	if (job_meta == nullptr) {
		throw job_exception("Job configuration cannot be null");
	}
	if (worker_conf == nullptr) {
		throw job_exception("Worker configuration cannot be null");
	}
	if (fs::exists(source_path) && fs::is_directory(source_path)) {
		throw job_exception("Source path does not exist or is not directory");
	}
	if (fileman == nullptr) {
		throw job_exception("File manager cannot be null");
	}


	// create fake task, which is logical root of evaluation
	size_t id = 0;
	std::map<std::string, size_t> eff_indegree;
	auto root_task = std::make_shared<fake_task>(id++);
	eff_indegree.insert(std::make_pair(root_task->get_task_id(), 0));


	// construct all tasks with their ids and check if they have all datas, but do not connect them
	std::map<std::string, std::shared_ptr<task_base>> unconnected_tasks;
	for (auto &task_meta : job_meta->tasks) {
		if (task_meta->task_id == "") {
			throw job_exception("Task ID cannot be empty");
		}
		if (task_meta->priority == 0) {
			throw job_exception("Priority cannot be zero");
		}
		if (task_meta->binary == "") {
			throw job_exception("Command cannot be empty");
		}

		// distinguish internal/external command and construct suitable object
		if (task_meta->sandbox != nullptr) {

			// //////////////// //
			// external command //
			// //////////////// //

			auto sandbox = task_meta->sandbox;
			std::shared_ptr<sandbox_limits> limits;
			auto worker_limits = worker_conf->get_limits();

			if (sandbox->name == "") {
				throw job_exception("Sandbox name cannot be empty");
			}

			// first we have to get propriate hwgroup limits
			bool hw_found = false;
			auto its = worker_conf->get_headers().equal_range("hwgroup");
			for (auto it = its.first; it != its.second; ++it) {
				auto hwit = sandbox->limits.find(it->second);
				if (hwit != sandbox->limits.end()) {
					limits = hwit->second;
					hw_found = true;
				}
			}
			if (!hw_found) {
				throw job_exception("Hwgroup with specified name not defined");
			}

			// we have to load defaults from worker_config if necessary
			if (limits->cpu_time == FLT_MAX) {
				limits->cpu_time = worker_limits.cpu_time;
			}
			if (limits->wall_time == FLT_MAX) {
				limits->wall_time = worker_limits.wall_time;
			}
			if (limits->extra_time == FLT_MAX) {
				limits->extra_time = worker_limits.extra_time;
			}
			if (limits->stack_size == SIZE_MAX) {
				limits->stack_size = worker_limits.stack_size;
			}
			if (limits->memory_usage == SIZE_MAX) {
				limits->memory_usage = worker_limits.memory_usage;
			}
			if (limits->processes == SIZE_MAX) {
				limits->processes = worker_limits.processes;
			}
			if (limits->disk_blocks == SIZE_MAX) {
				limits->disk_blocks = worker_limits.disk_blocks;
			}
			if (limits->disk_inodes == SIZE_MAX) {
				limits->disk_inodes = worker_limits.disk_inodes;
			}

			// ... and finally construct external task from given information
			limits->std_input = task_meta->std_input;
			limits->std_output = task_meta->std_output;
			limits->std_error = task_meta->std_error;
			std::shared_ptr<task_base> task = std::make_shared<external_task>(
						worker_conf->get_worker_id(), id++, task_meta->task_id, task_meta->priority,
						task_meta->fatal_failure, task_meta->dependencies, task_meta->binary,
						task_meta->cmd_args, sandbox->name, *limits);
			unconnected_tasks.insert(std::make_pair(task_meta->task_id, task));
			eff_indegree.insert(std::make_pair(task_meta->task_id, 0));

		} else {

			// //////////////// //
			// internal command //
			// //////////////// //

			std::shared_ptr<task_base> task;

			if (task_meta->binary == "cp") {
				task = std::make_shared<cp_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, "", task_meta->dependencies);
			} else if (task_meta->binary == "mkdir") {
				task = std::make_shared<mkdir_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, "", task_meta->dependencies);
			} else if (task_meta->binary == "rename") {
				task = std::make_shared<rename_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, "", task_meta->dependencies);
			} else if (task_meta->binary == "rm") {
				task = std::make_shared<rm_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, "", task_meta->dependencies);
			} else if (task_meta->binary == "archivate") {
				task = std::make_shared<archivate_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, "", task_meta->dependencies);
			} else if (task_meta->binary == "extract") {
				task = std::make_shared<extract_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, "", task_meta->dependencies);
			} else if (task_meta->binary == "fetch") {
				task = std::make_shared<fetch_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, "", task_meta->dependencies, fileman);
			} else {
				throw job_exception("Unknown internal task: " + task_meta->binary);
			}

			unconnected_tasks.insert(std::make_pair(task_meta->task_id, task));
			eff_indegree.insert(std::make_pair(task_meta->task_id, 0));
		}
	}


	// constructed tasks in map have to have tree structure, so... make it and connect them
	for (auto &elem : unconnected_tasks) {
		const std::vector<std::string> &depend = elem.second->get_dependencies();

		// connect all suitable task underneath root
		if (depend.size() == 0) {
			root_task->add_children(elem.second);
			elem.second->add_parent(root_task);
			eff_indegree.at(elem.first) = 1;
		} else {
			// write indegrees to every task
			eff_indegree.at(elem.first) = depend.size();
		}

		for (size_t i = 0; i < depend.size(); ++i) {
			try {
				auto ptr = unconnected_tasks.at(depend.at(i));
				ptr->add_children(elem.second);
				elem.second->add_parent(ptr);
			} catch (std::out_of_range) {
				throw job_exception("Non existing task-id (" + depend.at(i) + ") in dependency list");
			}
		}
	}


	// all should be done now... just linear ordering is missing...
	try {
		helpers::topological_sort(root_task, eff_indegree, task_queue_);
	} catch (helpers::top_sort_exception &e) {
		throw job_exception(e.what());
	}
}

job::~job()
{
	cleanup_job();
}

void job::run()
{
	// simply run all tasks in given topological order
	for (auto &i : task_queue_) {
		try {
			if (i->is_executable()) {
				i->run();
			} else {
				// we have to pass information about non-execution to children
				i->set_children_execution(false);
			}
		} catch (std::exception) {
			if (i->get_fatal_failure()) {
				break;
			} else {
				// set executable bit in this task and in children
				i->set_execution(false);
				i->set_children_execution(false);
			}
		}
	}

	return;
}

std::map<std::string, std::shared_ptr<task_results>> job::get_results()
{
	std::map<std::string, std::shared_ptr<task_results>> res;

	for (auto &i : task_queue_) {
		if (i->get_result() != nullptr) {
			res.emplace(i->get_task_id(), i->get_result());
		}
	}

	return res;
}

void job::cleanup_job()
{
	// destroy all files in working directory
	// -> job_evaluator will handle this for us...

	return;
}

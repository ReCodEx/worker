#include "job.h"
#include "job_exception.h"

job::job(std::shared_ptr<job_metadata> job_meta, std::shared_ptr<worker_config> worker_conf,
		 fs::path source_path, fs::path result_path, std::shared_ptr<file_manager_base> fileman)
	: job_meta_(job_meta), worker_config_(worker_conf), source_path_(source_path),
	  result_path_(result_path), fileman_(fileman)
{
	// check construction parameters if they are in right format
	if (job_meta_ == nullptr) {
		throw job_exception("Job configuration cannot be null");
	} else if (worker_config_ == nullptr) {
		throw job_exception("Worker configuration cannot be null");
	} else if (!fs::exists(source_path_)) {
		throw job_exception("Source code directory not exists");
	} else if (!fs::is_directory(source_path_)) {
		throw job_exception("Source code directory is not directory");
	} else if (fs::is_empty(source_path_)) {
		throw job_exception("Source code directory is empty");
	} else if (fileman_ == nullptr) {
		throw job_exception("File manager cannot be null");
	}

	// construct system logger for this job
	init_logger();

	// build job from given job configuration
	build_job();
}

job::~job()
{
	cleanup_job();
}

void job::build_job()
{
	// check job info
	if (job_meta_->job_id == "") {
		throw job_exception("Job ID cannot be empty");
	} else if (job_meta_->language == "") {
		throw job_exception("Language cannot be empty");
	} else if (job_meta_->file_server_url == "") {
		throw job_exception("File server URL cannot be empty");
	}


	// create fake task, which is logical root of evaluation
	size_t id = 0;
	std::map<std::string, size_t> eff_indegree;
	root_task_ = std::make_shared<fake_task>(id++);
	eff_indegree.insert(std::make_pair(root_task_->get_task_id(), 0));


	// construct all tasks with their ids and check if they have all datas, but do not connect them
	std::map<std::string, std::shared_ptr<task_base>> unconnected_tasks;
	for (auto &task_meta : job_meta_->tasks) {
		if (task_meta->task_id == "") {
			throw job_exception("Task ID cannot be empty");
		} else if (task_meta->priority == 0) {
			throw job_exception("Priority cannot be zero");
		} else if (task_meta->binary == "") {
			throw job_exception("Command cannot be empty");
		}

		// distinguish internal/external command and construct suitable object
		if (task_meta->sandbox != nullptr) {

			// //////////////// //
			// external command //
			// //////////////// //

			auto sandbox = task_meta->sandbox;
			std::shared_ptr<sandbox_limits> limits;
			auto worker_limits = worker_config_->get_limits();

			if (sandbox->name == "") {
				throw job_exception("Sandbox name cannot be empty");
			}

			// first we have to get propriate hwgroup limits
			bool hw_found = false;
			auto its = worker_config_->get_headers().equal_range("hwgroup");
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
			if (limits->disk_size == SIZE_MAX) {
				limits->disk_size = worker_limits.disk_size;
			}
			if (limits->disk_files == SIZE_MAX) {
				limits->disk_files = worker_limits.disk_files;
			}

			// ... and finally construct external task from given information
			limits->std_input = task_meta->std_input;
			limits->std_output = task_meta->std_output;
			limits->std_error = task_meta->std_error;
			std::shared_ptr<task_base> task = std::make_shared<external_task>(
						worker_config_->get_worker_id(), id++, task_meta->task_id, task_meta->priority,
						task_meta->fatal_failure, task_meta->dependencies, task_meta->binary,
						task_meta->cmd_args, sandbox->name, *limits, logger_);
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
					task_meta->cmd_args, task_meta->dependencies);
			} else if (task_meta->binary == "mkdir") {
				task = std::make_shared<mkdir_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, task_meta->dependencies);
			} else if (task_meta->binary == "rename") {
				task = std::make_shared<rename_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, task_meta->dependencies);
			} else if (task_meta->binary == "rm") {
				task = std::make_shared<rm_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, task_meta->dependencies);
			} else if (task_meta->binary == "archivate") {
				task = std::make_shared<archivate_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, task_meta->dependencies);
			} else if (task_meta->binary == "extract") {
				task = std::make_shared<extract_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, task_meta->dependencies);
			} else if (task_meta->binary == "fetch") {
				task = std::make_shared<fetch_task>(id++, task_meta->task_id, task_meta->priority,
					task_meta->fatal_failure, task_meta->binary,
					task_meta->cmd_args, task_meta->dependencies, fileman_);
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
			root_task_->add_children(elem.second);
			elem.second->add_parent(root_task_);
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
		helpers::topological_sort(root_task_, eff_indegree, task_queue_);
	} catch (helpers::top_sort_exception &e) {
		throw job_exception(e.what());
	}
}

std::vector<std::pair<std::string, std::shared_ptr<task_results>>> job::run()
{
	std::vector<std::pair<std::string, std::shared_ptr<task_results>>> results;

	// simply run all tasks in given topological order
	for (auto &i : task_queue_) {
		auto task_id = i->get_task_id();
		logger_->info() << "Running task " << task_id;
		try {
			if (i->is_executable()) {
				auto res = i->run();
				if (res == nullptr) {
					continue;
				}
				results.push_back({task_id, res});
			} else {
				// we have to pass information about non-execution to children
				logger_->info() << "Task not executable";
				i->set_children_execution(false);
			}
		} catch (std::exception &e) {
			std::shared_ptr<task_results> result(new task_results());
			result->failed = true;
			result->error_message = e.what();
			results.push_back({task_id, result});
			if (i->get_fatal_failure()) {
				logger_->info() << "Task failed - fatal failure";
				break;
			} else {
				// set executable bit in this task and in children
				logger_->info() << "Task failed - childs will not be executed";
				i->set_execution(false);
				i->set_children_execution(false);
			}
		}
		logger_->info() << "Task " << task_id << " finished";
	}

	return results;
}

void job::init_logger()
{
	std::string log_name = "job_system_log.log";
	spdlog::level::level_enum log_level = spdlog::level::debug;

	//Create and register logger
	try {
		//Create multithreaded rotating file sink. Max filesize is 1024 * 1024 and we save 5 newest files.
		auto file_sink = std::make_shared<spdlog::sinks::simple_file_sink_mt>((result_path_ / log_name).string(), true);
		//Set queue size for asynchronous logging. It must be a power of 2.
		spdlog::set_async_mode(1048576);
		//Make log with name "logger"
		auto file_logger = std::make_shared<spdlog::logger>("logger", file_sink);
		//Set logging level to debug
		file_logger->set_level(log_level);
		//Print header to log
		file_logger->info() << "------------------------------";
		file_logger->info() << "       Job system log";
		file_logger->info() << "------------------------------";
		logger_ = file_logger;
	} catch (spdlog::spdlog_ex &e) {
		//Suppose not happen. But in case, create only empty logger.
		auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
		logger_ = std::make_shared<spdlog::logger>("job_manager_nolog", sink);
	}
}

void job::cleanup_job()
{
	// destroy all files in working directory
	// -> job_evaluator will handle this for us...

	return;
}

const std::vector<std::shared_ptr<task_base>> &job::get_task_queue() const
{
	return task_queue_;
}

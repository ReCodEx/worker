#include "job.h"
#include "job_exception.h"

job::job(std::shared_ptr<job_metadata> job_meta,
	std::shared_ptr<worker_config> worker_conf,
	fs::path working_directory,
	fs::path source_path,
	fs::path result_path,
	std::shared_ptr<file_manager_base> fileman)
	: job_meta_(job_meta), worker_config_(worker_conf), working_directory_(working_directory),
	  source_path_(source_path), result_path_(result_path), fileman_(fileman)
{
	// check construction parameters if they are in right format
	if (job_meta_ == nullptr) {
		throw job_exception("Job configuration cannot be null");
	} else if (worker_config_ == nullptr) {
		throw job_exception("Worker configuration cannot be null");
	} else if (fileman_ == nullptr) {
		throw job_exception("File manager cannot be null");
	}

	// check injected directories
	check_job_dirs();

	// prepare variables which will be used in job config
	prepare_job_vars();

	// construct system logger for this job
	init_logger();

	// build job from given job configuration
	build_job();
}

job::~job()
{
	cleanup_job();
}

void job::check_job_dirs()
{
	if (!fs::exists(working_directory_)) {
		throw job_exception("Working directory not exists");
	} else if (!fs::is_directory(working_directory_)) {
		throw job_exception("Working directory is not directory");
	}

	if (!fs::exists(source_path_)) {
		throw job_exception("Source code directory not exists");
	} else if (!fs::is_directory(source_path_)) {
		throw job_exception("Source code directory is not directory");
	} else if (fs::is_empty(source_path_)) {
		throw job_exception("Source code directory is empty");
	}

	// check result files directory
	if (!fs::exists(result_path_)) {
		throw job_exception("Result files directory not exists");
	} else if (!fs::is_directory(result_path_)) {
		throw job_exception("Result files directory is not directory");
	}
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


	// create root task, which is logical root of evaluation
	size_t id = 0;
	std::map<std::string, size_t> eff_indegree;
	root_task_ = std::make_shared<root_task>(id++);
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

		// go through variables parsing
		task_meta->binary = parse_job_var(task_meta->binary);
		for (size_t i = 0; i < task_meta->cmd_args.size(); ++i) {
			task_meta->cmd_args.at(i) = parse_job_var(task_meta->cmd_args.at(i));
		}

		// distinguish internal/external command and construct suitable object
		if (task_meta->sandbox != nullptr) {

			// //////////////// //
			// external command //
			// //////////////// //

			auto sandbox = task_meta->sandbox;

			if (sandbox->name == "") {
				throw job_exception("Sandbox name cannot be empty");
			}

			// first we have to get appropriate hwgroup limits
			std::shared_ptr<sandbox_limits> limits;
			auto hwit = sandbox->loaded_limits.find(worker_config_->get_hwgroup());
			if (hwit != sandbox->loaded_limits.end()) {
				limits = hwit->second;
			} else {
				throw job_exception("Worker hwgroup was not found in loaded limits");
			}

			// check and maybe modify limits
			process_task_limits(limits);

			// go through variables parsing
			limits->std_input = parse_job_var(task_meta->std_input);
			limits->std_output = parse_job_var(task_meta->std_output);
			limits->std_error = parse_job_var(task_meta->std_error);
			limits->chdir = parse_job_var(limits->chdir);
			std::vector<std::tuple<std::string, std::string, sandbox_limits::dir_perm>> new_bnd_dirs;
			for (auto &bnd_dir : limits->bound_dirs) {
				new_bnd_dirs.push_back(std::tuple<std::string, std::string, sandbox_limits::dir_perm>{
					parse_job_var(std::get<0>(bnd_dir)), parse_job_var(std::get<1>(bnd_dir)), std::get<2>(bnd_dir)});
			}
			limits->bound_dirs = new_bnd_dirs;

			// ... and finally construct external task from given information
			external_task::create_params data = {
				worker_config_->get_worker_id(), id++, task_meta, limits, logger_, working_directory_.string()};
			std::shared_ptr<task_base> task = std::make_shared<external_task>(data);
			unconnected_tasks.insert(std::make_pair(task_meta->task_id, task));
			eff_indegree.insert(std::make_pair(task_meta->task_id, 0));

		} else {

			// //////////////// //
			// internal command //
			// //////////////// //

			std::shared_ptr<task_base> task;

			if (task_meta->binary == "cp") {
				task = std::make_shared<cp_task>(id++, task_meta);
			} else if (task_meta->binary == "mkdir") {
				task = std::make_shared<mkdir_task>(id++, task_meta);
			} else if (task_meta->binary == "rename") {
				task = std::make_shared<rename_task>(id++, task_meta);
			} else if (task_meta->binary == "rm") {
				task = std::make_shared<rm_task>(id++, task_meta);
			} else if (task_meta->binary == "archivate") {
				task = std::make_shared<archivate_task>(id++, task_meta);
			} else if (task_meta->binary == "extract") {
				task = std::make_shared<extract_task>(id++, task_meta);
			} else if (task_meta->binary == "fetch") {
				task = std::make_shared<fetch_task>(id++, task_meta, fileman_);
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

void job::process_task_limits(std::shared_ptr<sandbox_limits> limits)
{
	if (limits == nullptr) {
		throw job_exception("Internal error. Nullptr dereference in process_task_limits.");
	}

	auto worker_limits = worker_config_->get_limits();
	std::string msg = " item is bigger than default worker value";

	// we have to load defaults from worker_config if necessary
	// and check for bigger limits than in worker_config
	if (limits->cpu_time == FLT_MAX) {
		limits->cpu_time = worker_limits.cpu_time;
	} else {
		if (limits->cpu_time > worker_limits.cpu_time) {
			throw job_exception("time" + msg);
		}
	}
	if (limits->wall_time == FLT_MAX) {
		limits->wall_time = worker_limits.wall_time;
	} else {
		if (limits->wall_time > worker_limits.wall_time) {
			throw job_exception("wall-time" + msg);
		}
	}
	if (limits->extra_time == FLT_MAX) {
		limits->extra_time = worker_limits.extra_time;
	} else {
		if (limits->extra_time > worker_limits.extra_time) {
			throw job_exception("extra-time" + msg);
		}
	}
	if (limits->stack_size == SIZE_MAX) {
		limits->stack_size = worker_limits.stack_size;
	} else {
		if (limits->stack_size > worker_limits.stack_size) {
			throw job_exception("stack-size" + msg);
		}
	}
	if (limits->memory_usage == SIZE_MAX) {
		limits->memory_usage = worker_limits.memory_usage;
	} else {
		if (limits->memory_usage > worker_limits.memory_usage) {
			throw job_exception("memory" + msg);
		}
	}
	if (limits->processes == SIZE_MAX) {
		limits->processes = worker_limits.processes;
	} else {
		if (limits->processes > worker_limits.processes) {
			throw job_exception("parallel" + msg);
		}
	}
	if (limits->disk_size == SIZE_MAX) {
		limits->disk_size = worker_limits.disk_size;
	} else {
		if (limits->disk_size > worker_limits.disk_size) {
			throw job_exception("disk-size" + msg);
		}
	}
	if (limits->disk_files == SIZE_MAX) {
		limits->disk_files = worker_limits.disk_files;
	} else {
		if (limits->disk_files > worker_limits.disk_files) {
			throw job_exception("disk-files" + msg);
		}
	}

	// union of bound directories and environs from worker configuration and job configuration
	limits->environ_vars.insert(
		limits->environ_vars.end(), worker_limits.environ_vars.begin(), worker_limits.environ_vars.end());
	limits->bound_dirs.insert(
		limits->bound_dirs.end(), worker_limits.bound_dirs.begin(), worker_limits.bound_dirs.end());
}

std::vector<std::pair<std::string, std::shared_ptr<task_results>>> job::run()
{
	std::vector<std::pair<std::string, std::shared_ptr<task_results>>> results;

	// simply run all tasks in given topological order
	for (auto &i : task_queue_) {
		// we don't want nullptr dereference
		if (i == nullptr) {
			continue;
		}

		auto task_id = i->get_task_id();
		try {
			if (i->is_executable()) {
				auto res = i->run();
				logger_->info() << "Task \"" << task_id << "\" ran successfully";

				// if task has some results than publish them in output
				if (res != nullptr) {
					results.push_back({task_id, res});
				}
			} else {
				// we have to pass information about non-execution to children
				logger_->info() << "Task \"" << task_id << "\" marked as not executable, proceeding to next task";
				i->set_children_execution(false);
			}
		} catch (std::exception &e) {
			std::shared_ptr<task_results> result(new task_results());
			result->failed = true;
			result->error_message = e.what();
			results.push_back({task_id, result});

			logger_->info() << "Task \"" << task_id << "\" failed: " << e.what();

			if (i->get_fatal_failure()) {
				logger_->info() << "Fatal failure bit set. Terminating of job execution...";
				break;
			} else {
				// set executable bit in this task and in children
				logger_->info() << "Task children will not be executed";
				i->set_execution(false);
				i->set_children_execution(false);
			}
		}
	}

	return results;
}

void job::init_logger()
{
	if (!job_meta_->log) {
		// logging is disabled in configuration
		logger_ = helpers::create_null_logger();
		return;
	}

	std::string log_name = "job_system_log.log";
	spdlog::level::level_enum log_level = spdlog::level::debug;

	// Create and register logger
	try {
		// Create multithreaded rotating file sink. Max filesize is 1024 * 1024 and we save 5 newest files.
		auto file_sink = std::make_shared<spdlog::sinks::simple_file_sink_mt>((result_path_ / log_name).string(), true);
		// Set queue size for asynchronous logging. It must be a power of 2.
		spdlog::set_async_mode(1048576);
		// Make log with name "logger"
		auto file_logger = std::make_shared<spdlog::logger>("logger", file_sink);
		// Set logging level to debug
		file_logger->set_level(log_level);
		// Print header to log
		file_logger->info() << "------------------------------";
		file_logger->info() << "       Job system log";
		file_logger->info() << "------------------------------";
		logger_ = file_logger;
	} catch (spdlog::spdlog_ex) {
		// Suppose not happen. But in case, create only empty logger.
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

void job::prepare_job_vars()
{
	// define and fill variables which can be used within job configuration
	job_variables_ = {{"WORKER_ID", std::to_string(worker_config_->get_worker_id())},
		{"JOB_ID", job_meta_->job_id},
		{"SOURCE_DIR", source_path_.string()},
		{"EVAL_DIR", fs::path("/evaluate").string()},
		{"RESULT_DIR", result_path_.string()},
		{"TEMP_DIR", fs::temp_directory_path().string()},
		{"JUDGES_DIR", fs::path("/usr/bin").string()}};

	return;
}

std::string job::parse_job_var(const std::string &src)
{
	std::string res = src;

	size_t start = 0;
	while ((start = res.find("${", start)) != std::string::npos) {
		size_t end = res.find("}", start + 1);
		size_t len = end - start - 2;
		if (end == std::string::npos) {
			throw job_exception("Not closed variable name: " + res.substr(start));
		}

		if (job_variables_.find(res.substr(start + 2, len)) != job_variables_.end()) {
			// we found variable and can replace it in string
			res.replace(start, end - start + 1, job_variables_.at(res.substr(start + 2, len)));
		}

		// start++; // just to be sure we're not in cycle
	}

	return res;
}

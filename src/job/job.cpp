#include "job.h"
#include "job_exception.h"

job::job(std::shared_ptr<job_metadata> job_meta,
	std::shared_ptr<worker_config> worker_conf,
	fs::path working_directory,
	fs::path source_path,
	fs::path result_path,
	std::shared_ptr<task_factory_interface> factory,
	std::shared_ptr<progress_callback_interface> progr_callback)
	: job_meta_(job_meta), worker_config_(worker_conf), working_directory_(working_directory),
	  source_path_(source_path), result_path_(result_path), factory_(factory), progress_callback_(progr_callback)
{
	// check construction parameters if they are in right format
	if (job_meta_ == nullptr) {
		throw job_exception("Job configuration cannot be null");
	} else if (worker_config_ == nullptr) {
		throw job_exception("Worker configuration cannot be null");
	} else if (factory_ == nullptr) {
		throw job_exception("Task factory pointer cannot be null");
	}

	// if progress callback is null, we have to use default one
	init_progress_callback();

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
	} else if (job_meta_->file_server_url == "") {
		throw job_exception("File server URL cannot be empty");
	} else if (job_meta_->hwgroups.empty()) {
		throw job_exception("Job configuration has no specified hwgroup");
	}

	// check if job is meant to be processed on this worker
	auto hw_group_it = std::find(job_meta_->hwgroups.begin(), job_meta_->hwgroups.end(), worker_config_->get_hwgroup());
	if (hw_group_it == job_meta_->hwgroups.end()) {
		throw job_exception("Job is not supposed to be processed on this worker, hwgroups does not match");
	}

	// create root task, which is logical root of evaluation
	size_t id = 0;
	root_task_ = factory_->create_internal_task(id++);

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

		std::shared_ptr<task_base> task;

		// distinguish internal/external command and construct suitable object
		if (task_meta->sandbox != nullptr) {

			// //////////////// //
			// external command //
			// //////////////// //

			auto sandbox = task_meta->sandbox;

			if (sandbox->name.empty()) {
				throw job_exception("Sandbox name cannot be empty");
			}

			// first we have to get appropriate hwgroup limits
			std::shared_ptr<sandbox_limits> limits;
			auto hwit = sandbox->loaded_limits.find(worker_config_->get_hwgroup());
			if (hwit != sandbox->loaded_limits.end()) {
				limits = hwit->second;

				// check and maybe modify limits
				process_task_limits(limits);
			} else {
				limits = std::make_shared<sandbox_limits>(worker_config_->get_limits());
			}

			// go through variables parsing
			sandbox->std_input = parse_job_var(sandbox->std_input);
			sandbox->std_output = parse_job_var(sandbox->std_output);
			sandbox->std_error = parse_job_var(sandbox->std_error);
			limits->chdir = parse_job_var(limits->chdir);
			std::vector<std::tuple<std::string, std::string, sandbox_limits::dir_perm>> new_bnd_dirs;
			for (auto &bnd_dir : limits->bound_dirs) {
				new_bnd_dirs.push_back(std::tuple<std::string, std::string, sandbox_limits::dir_perm>{
					parse_job_var(std::get<0>(bnd_dir)), parse_job_var(std::get<1>(bnd_dir)), std::get<2>(bnd_dir)});
			}
			limits->bound_dirs = new_bnd_dirs;

			// ... and finally construct external task from given information
			create_params data = {worker_config_->get_worker_id(),
				id++,
				task_meta,
				sandbox,
				limits,
				logger_,
				working_directory_.string()};

			task = factory_->create_sandboxed_task(data);

		} else {

			// //////////////// //
			// internal command //
			// //////////////// //

			task = factory_->create_internal_task(id++, task_meta);

			if (task == nullptr) {
				throw job_exception("Unknown internal task: " + task_meta->binary);
			}
		}

		// add newly created task to container ready for connect with other tasks
		unconnected_tasks.insert(std::make_pair(task_meta->task_id, task));
	}

	// constructed tasks in map have to have tree structure, so... make it and connect them
	connect_tasks(root_task_, unconnected_tasks);

	// all should be done now... just linear ordering is missing...
	try {
		helpers::topological_sort(root_task_, task_queue_);
	} catch (helpers::top_sort_exception &e) {
		throw job_exception(e.what());
	}

	// remove unnecessary root task from begining of task queue
	if (!task_queue_.empty() && task_queue_.at(0)->get_task_id() == "") {
		task_queue_.erase(task_queue_.begin());
	} else {
		// something bad is happening here, stop this job evaluation
		throw job_exception("Root task not present in first place after topological sort.");
	}
}

void job::process_task_limits(std::shared_ptr<sandbox_limits> limits)
{
	if (limits == nullptr) {
		throw job_exception("Internal error. Nullptr dereference in process_task_limits.");
	}

	auto worker_limits = worker_config_->get_limits();
	std::string msg = " item is bigger than default worker value";

	// we have to load defaults from worker_config if necessary and check for bigger limits than in worker_config
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

void job::connect_tasks(
	std::shared_ptr<task_base> root, std::map<std::string, std::shared_ptr<task_base>> &unconn_tasks)
{
	for (auto &elem : unconn_tasks) {
		const std::vector<std::string> &depend = elem.second->get_dependencies();

		// connect all suitable task underneath root
		if (depend.size() == 0) {
			root->add_children(elem.second);
			elem.second->add_parent(root);
		}

		for (size_t i = 0; i < depend.size(); ++i) {
			try {
				auto ptr = unconn_tasks.at(depend.at(i));
				ptr->add_children(elem.second);
				elem.second->add_parent(ptr);
			} catch (std::out_of_range) {
				throw job_exception("Non existing task-id (" + depend.at(i) + ") in dependency list");
			}
		}
	}
}

std::vector<std::pair<std::string, std::shared_ptr<task_results>>> job::run()
{
	std::vector<std::pair<std::string, std::shared_ptr<task_results>>> results;
	progress_callback_->job_started(job_meta_->job_id);

	// simply run all tasks in given topological order
	for (auto &task : task_queue_) {
		// we don't want nullptr dereference
		if (task == nullptr) {
			continue;
		}

		auto task_id = task->get_task_id();
		try {
			if (task->is_executable()) {
				auto res = task->run();
				logger_->info("Task \"{}\" ran successfully", task_id);
				progress_callback_->task_completed(job_meta_->job_id, task_id);

				// if task has some results than publish them in output
				if (res != nullptr) {
					results.push_back({task_id, res});
				}
			} else {
				logger_->info("Task \"{}\" marked as not executable, proceeding to next task", task_id);
				progress_callback_->task_skipped(job_meta_->job_id, task_id);

				// even skipped task has its own result entry
				std::shared_ptr<task_results> result(new task_results());
				result->status = task_status::SKIPPED;
				results.push_back({task_id, result});

				// we have to pass information about non-execution to children
				task->set_children_execution(false);
			}
		} catch (std::exception &e) {
			if (task->get_type() == task_type::INNER) {
				// evaluation just encountered internal error and its quite possible
				// that something is very wrong in here, so be gentle and crash like a sir
				// and try not to mess up next job execution
				throw;
			}

			std::shared_ptr<task_results> result(new task_results());
			result->status = task_status::FAILED;
			result->error_message = e.what();
			results.push_back({task_id, result});

			logger_->info("Task \"{}\" failed: {}", task_id, e.what());
			progress_callback_->task_failed(job_meta_->job_id, task_id);

			if (task->get_fatal_failure()) {
				logger_->info("Fatal failure bit set. Terminating of job execution...");
				break;
			} else {
				// set executable bit in this task and in children
				logger_->info("Task children will not be executed");
				task->set_execution(false);
				task->set_children_execution(false);
			}
		}
	}

	progress_callback_->job_ended(job_meta_->job_id);
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
		file_logger->info("------------------------------");
		file_logger->info("       Job system log");
		file_logger->info("------------------------------");
		logger_ = file_logger;
	} catch (spdlog::spdlog_ex) {
		// Suppose not happen. But in case, create only empty logger.
		logger_ = helpers::create_null_logger();
	}
}

void job::init_progress_callback()
{
	if (progress_callback_ == nullptr) {
		progress_callback_ = std::make_shared<empty_progress_callback>();
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

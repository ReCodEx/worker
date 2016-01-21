#include "job.h"
#include "job_exception.h"


job::job(std::shared_ptr<job_tasks> tasks, fs::path source_path): source_path_(source_path)
{
	// check construction parameters if they are in right format
	if (tasks == nullptr) {
		throw job_exception("Task queue cannot be null");
	}

	task_queue_ = tasks->get_tasks();

	// check source code directory
	if (!fs::exists(source_path_)) {
		throw job_exception("Source code directory does not exist");
	} else if (!fs::is_directory(source_path_)) {
		throw job_exception("Source code directory is not a directory");
	} else if (fs::is_empty(source_path_)) {
		throw job_exception("Source code directory is empty");
	}

	// construct system logger for this job
	init_logger();

	// prepare job for evaluation
	prepare_job();
}

job::~job()
{
	cleanup_job();
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
	} catch(spdlog::spdlog_ex &e) {
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

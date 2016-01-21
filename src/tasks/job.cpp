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

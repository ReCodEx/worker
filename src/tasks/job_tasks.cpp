#include "job_tasks.h"
#include "job_exception.h"
#include "external_task.h"
#include "fake_task.h"
#include "internal/cp_task.h"
#include "internal/mkdir_task.h"
#include "internal/rename_task.h"
#include "internal/rm_task.h"
#include "internal/archivate_task.h"
#include "internal/extract_task.h"
#include "internal/fetch_task.h"
#include "../helpers/topological_sort.h"

job_tasks::job_tasks(
	const YAML::Node &config,
	std::shared_ptr<worker_config> worker_config,
	std::shared_ptr<file_manager_base> fileman
) : worker_config_(worker_config)
{

	// create fake task, which is logical root of evaluation
	size_t id = 0;
	std::map<std::string, size_t> eff_indegree;
	auto root_task = std::make_shared<fake_task>(id++);
	eff_indegree.insert(std::make_pair(root_task->get_task_id(), 0));


	// construct all tasks with their ids and check if they have all datas, but do not connect them
	std::map<std::string, std::shared_ptr<task_base>> unconnected_tasks;
	for (auto &ctask : config["tasks"]) {
		std::string task_id;
		size_t priority;
		bool fatal;
		std::string cmd;
		std::vector<std::string> args;
		std::string log;
		std::vector<std::string> task_depend;

		if (ctask["task-id"] && ctask["task-id"].IsScalar()) {
			task_id = ctask["task-id"].as<std::string>();
		} else { throw job_exception("Configuration task has missing task-id"); }
		if (ctask["priority"] && ctask["priority"].IsScalar()) {
			priority = ctask["priority"].as<size_t>();
		} else { throw job_exception("Configuration task has missing priority"); }
		if (ctask["fatal-failure"] && ctask["fatal-failure"].IsScalar()) {
			fatal = ctask["fatal-failure"].as<bool>();
		} else { throw job_exception("Configuration task has missing fatal-failure"); }
		if (ctask["cmd"]) {
			if (ctask["cmd"].IsMap()) {
				if (ctask["cmd"]["bin"] && ctask["cmd"]["bin"].IsScalar()) {
					cmd = ctask["cmd"]["bin"].as<std::string>();
				} else { throw job_exception("Runnable binary for task not given"); }

				if (ctask["cmd"]["args"] && ctask["cmd"]["args"].IsSequence()) {
					args = ctask["cmd"]["args"].as<std::vector<std::string>>();
				} // can be omitted... no throw
			} else { throw job_exception("Command in task is not a map"); }
		} else { throw job_exception("Configuration of one task has missing cmd"); }
		if (ctask["log"] && ctask["log"].IsScalar()) {
			log = ctask["log"].as<std::string>();
		} // can be omitted... no throw

		// load dependencies
		if (ctask["dependencies"] && ctask["dependencies"].IsSequence()) {
			task_depend = ctask["dependencies"].as<std::vector<std::string>>();
		}

		// distinguish internal/external command and construct suitable object
		if (ctask["sandbox"]) {

			// //////////////// //
			// external command //
			// //////////////// //

			std::string std_input;
			std::string std_output;
			std::string std_error;
			std::string sandbox_name;
			std::map<std::string, sandbox_limits> hwgroups;
			sandbox_limits limits;

			if (ctask["sandbox"]["name"] && ctask["sandbox"]["name"].IsScalar()) {
				sandbox_name = ctask["sandbox"]["name"].as<std::string>();
			} else { throw job_exception("Name of sandbox not given"); }

			if (ctask["stdin"] && ctask["stdin"].IsScalar()) {
				std_input = ctask["stdin"].as<std::string>();
			} // can be ommited... no throw
			if (ctask["stdout"] && ctask["stdout"].IsScalar()) {
				std_output = ctask["stdout"].as<std::string>();
			} // can be ommited... no throw
			if (ctask["stderr"] && ctask["stderr"].IsScalar()) {
				std_error = ctask["stderr"].as<std::string>();
			} // can be ommited... no throw

			// load limits... if they are supplied
			if (ctask["sandbox"]["limits"]) {
				if (!ctask["sandbox"]["limits"].IsSequence()) {
					throw job_exception("Sandbox limits are not sequence");
				}

				for (auto &lim : ctask["sandbox"]["limits"]) {
					sandbox_limits sl;
					std::string hwgroup;

					if (lim["hw-group-id"] && lim["hw-group-id"].IsScalar()) {
						hwgroup = lim["hw-group-id"].as<std::string>();
					} else { throw job_exception("Hwgroup ID not defined in sandbox limits"); }

					if (lim["time"] && lim["time"].IsScalar()) {
						sl.cpu_time = lim["time"].as<float>();
					} else { // if not defined, load from default config
						sl.cpu_time = worker_config->get_limits().cpu_time;
					}
					if (lim["wall-time"] && lim["wall-time"].IsScalar()) {
						sl.wall_time = lim["wall-time"].as<float>();
					} else { // if not defined, load from default config
						sl.wall_time = worker_config->get_limits().wall_time;
					}
					if (lim["extra-time"] && lim["extra-time"].IsScalar()) {
						sl.extra_time = lim["extra-time"].as<float>();
					} else { // if not defined, load from default config
						sl.extra_time = worker_config->get_limits().extra_time;
					}
					if (lim["stack-size"] && lim["stack-size"].IsScalar()) {
						sl.stack_size = lim["stack-size"].as<size_t>();
					} else { // if not defined, load from default config
						sl.stack_size = worker_config->get_limits().stack_size;
					}
					if (lim["memory"] && lim["memory"].IsScalar()) {
						sl.memory_usage = lim["memory"].as<size_t>();
					} else { // if not defined, load from default config
						sl.memory_usage = worker_config->get_limits().memory_usage;
					}
					if (lim["parallel"] && lim["parallel"].IsScalar()) { // TODO not defined properly
						lim["parallel"].as<bool>();
					} // can be omitted... no throw
					if (lim["disk-blocks"] && lim["disk-blocks"].IsScalar()) {
						sl.disk_blocks = lim["disk-blocks"].as<size_t>();
					} else { // if not defined, load from default config
						sl.disk_blocks = worker_config->get_limits().disk_blocks;
					}
					if (lim["disk-inodes"] && lim["disk-inodes"].IsScalar()) {
						sl.disk_inodes = lim["disk-inodes"].as<size_t>();
					} else { // if not defined, load from default config
						sl.disk_inodes = worker_config->get_limits().disk_inodes;
					}
					if (lim["chdir"] && lim["chdir"].IsScalar()) {
						sl.chdir = lim["chdir"].as<std::string>();
					} // can be omitted... no throw

					if (lim["bound-directories"] && lim["bound-directories"].IsMap()) {
						sl.bound_dirs = lim["bound-directories"].as<std::map<std::string, std::string>>();
					} // can be omitted... no throw

					if (lim["environ-variable"] && lim["environ-variable"].IsMap()) {
						for (auto &var : lim["environ-variable"]) {
							sl.environ_vars.insert(std::make_pair(var.first.as<std::string>(), var.second.as<std::string>()));
						}
					}

					hwgroups.insert(std::make_pair(hwgroup, sl));
				}
			}

			// and finally construct external task from given information
			// first we have to get propriate hwgroup limits
			bool hw_found = false;
			auto its = worker_config->get_headers().equal_range("hwgroup");
			for (auto it = its.first; it != its.second; ++it) {
				auto hwit = hwgroups.find(it->second);
				if (hwit != hwgroups.end()) {
					limits = hwit->second;
					hw_found = true;
				}
			}
			if (!hw_found) {
				throw job_exception("Hwgroup with specified name not defined");
			}

			limits.std_input = std_input;
			limits.std_output = std_output;
			limits.std_error = std_error;
			std::shared_ptr<task_base> task = std::make_shared<external_task>(
				worker_config->get_worker_id(), id++, task_id, priority, fatal,
				task_depend, cmd, args, sandbox_name, limits
			);
			unconnected_tasks.insert(std::make_pair(task_id, task));
			eff_indegree.insert(std::make_pair(task_id, 0));

		} else {

			// //////////////// //
			// internal command //
			// //////////////// //

			std::shared_ptr<task_base> task;

			if (cmd == "cp") {
				task = std::make_shared<cp_task>(id++, task_id, priority, fatal, cmd, args, log, task_depend);
			} else if (cmd == "mkdir") {
				task = std::make_shared<mkdir_task>(id++, task_id, priority, fatal, cmd, args, log, task_depend);
			} else if (cmd == "rename") {
				task = std::make_shared<rename_task>(id++, task_id, priority, fatal, cmd, args, log, task_depend);
			} else if (cmd == "rm") {
				task = std::make_shared<rm_task>(id++, task_id, priority, fatal, cmd, args, log, task_depend);
			} else if (cmd == "archivate") {
				task = std::make_shared<archivate_task>(id++, task_id, priority, fatal, cmd, args, log, task_depend);
			} else if (cmd == "extract") {
				task = std::make_shared<extract_task>(id++, task_id, priority, fatal, cmd, args, log, task_depend);
			} else if (cmd == "fetch") {
				task = std::make_shared<fetch_task>(id++, task_id, priority, fatal, cmd, args, log, task_depend, fileman);
			} else {
				throw job_exception("Unknown internal task: " + cmd);
			}

			unconnected_tasks.insert(std::make_pair(task_id, task));
			eff_indegree.insert(std::make_pair(task_id, 0));
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
		helpers::topological_sort(root_task, eff_indegree, tasks_);
	} catch (helpers::top_sort_exception &e) {
		throw job_exception(e.what());
	}
}

std::vector<std::shared_ptr<task_base>> job_tasks::get_tasks()
{
	return tasks_;
}

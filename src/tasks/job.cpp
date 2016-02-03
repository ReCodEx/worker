#include "job.h"

job::job(const YAML::Node &job_config, fs::path source_path, fs::path result_path,
		 std::shared_ptr<worker_config> default_config, std::shared_ptr<file_manager_base> fileman)
	: source_path_(source_path), result_path_(result_path), fileman_(fileman), root_task_(nullptr),
	  default_config_(default_config), logger_(nullptr)
{
	// check construction parameters if they are in right format
	if (default_config_ == nullptr) {
		throw job_exception("Default worker config not given");
	} else if (fileman_ == nullptr) {
		throw job_exception("Filemanager not given");
	}

	// check source code directory
	if (!fs::exists(source_path_)) {
		throw job_exception("Source code directory not exists");
	} else if (!fs::is_directory(source_path_)) {
		throw job_exception("Source code directory is not directory");
	} else if (fs::is_empty(source_path_)) {
		throw job_exception("Source code directory is empty");
	}

	// construct system logger for this job
	init_logger();

	// build job
	build_job(job_config);

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

void job::build_job(const YAML::Node &conf)
{
	try {
		// initial checkouts
		if (!conf.IsDefined()) {
			throw job_exception("Job config file was empty");
		} else if (!conf.IsMap()) {
			throw job_exception("Job configuration is not a map");
		} else if (!conf["tasks"]) {
			throw job_exception("Item tasks was not given in job configuration");
		} else if (!conf["submission"]) {
			throw job_exception("Item submission was not given in job configuration");
		} else if (!conf["tasks"].IsSequence()) {
			throw job_exception("Item tasks in job configuration is not sequence");
		} else if (!conf["submission"].IsMap()) {
			throw job_exception("Item submission in job configuration is not map");
		}


		// get information about submission
		auto submiss = conf["submission"];
		if (submiss["job-id"] && submiss["job-id"].IsScalar()) {
			job_id_ = submiss["job-id"].as<std::string>();
		} else { throw job_exception("Submission.job-id item not loaded properly"); }
		if (submiss["language"] && submiss["language"].IsScalar()) {
			language_ = submiss["language"].as<std::string>();
		} else { throw job_exception("Submission.language item not loaded properly"); }
		if (submiss["file-collector"]) {
			auto filecol = submiss["file-collector"];
			if (filecol.IsMap()) {
				if (filecol["hostname"] && filecol["hostname"].IsScalar()) {
					fileman_hostname_ = filecol["hostname"].as<std::string>();
				} else { throw job_exception("Submission.file-collector.hostname item not loaded properly"); }
				if (filecol["port"] && filecol["port"].IsScalar()) {
					fileman_port_ = filecol["port"].as<std::string>();
				} else { throw job_exception("Submission.file-collector.port item not loaded properly"); }
				if (filecol["username"] && filecol["username"].IsScalar()) {
					fileman_username_ = filecol["username"].as<std::string>();
				} else { throw job_exception("Submission.file-collector.username item not loaded properly"); }
				if (filecol["password"] && filecol["password"].IsScalar()) {
					fileman_passwd_ = filecol["password"].as<std::string>();
				} else { throw job_exception("Submission.file-collector.password item not loaded properly"); }
			} else {
				throw job_exception("Item submission.file-collector is not map");
			}
		} else { throw job_exception("Submission item not loaded properly"); }


		// create fake task, which is logical root of evaluation
		size_t id = 0;
		std::map<std::string, size_t> eff_indegree;
		root_task_ = std::make_shared<fake_task>(id++);
		eff_indegree.insert(std::make_pair(root_task_->get_task_id(), 0));


		// construct all tasks with their ids and check if they have all datas, but do not connect them
		std::map<std::string, std::shared_ptr<task_base>> unconnected_tasks;
		for (auto &ctask : conf["tasks"]) {
			std::string task_id;
			size_t priority;
			bool fatal;
			std::string cmd;
			std::vector<std::string> args;
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
							sl.cpu_time = default_config_->get_limits().cpu_time;
						}
						if (lim["wall-time"] && lim["wall-time"].IsScalar()) {
							sl.wall_time = lim["wall-time"].as<float>();
						} else { // if not defined, load from default config
							sl.wall_time = default_config_->get_limits().wall_time;
						}
						if (lim["extra-time"] && lim["extra-time"].IsScalar()) {
							sl.extra_time = lim["extra-time"].as<float>();
						} else { // if not defined, load from default config
							sl.extra_time = default_config_->get_limits().extra_time;
						}
						if (lim["stack-size"] && lim["stack-size"].IsScalar()) {
							sl.stack_size = lim["stack-size"].as<size_t>();
						} else { // if not defined, load from default config
							sl.stack_size = default_config_->get_limits().stack_size;
						}
						if (lim["memory"] && lim["memory"].IsScalar()) {
							sl.memory_usage = lim["memory"].as<size_t>();
						} else { // if not defined, load from default config
							sl.memory_usage = default_config_->get_limits().memory_usage;
						}
						if (lim["parallel"] && lim["parallel"].IsScalar()) { // TODO not defined properly
							lim["parallel"].as<bool>();
						} // can be omitted... no throw
						if (lim["disk-blocks"] && lim["disk-blocks"].IsScalar()) {
							sl.disk_size = lim["disk-size"].as<size_t>();
						} else { // if not defined, load from default config
							sl.disk_size = default_config_->get_limits().disk_size;
						}
						if (lim["disk-files"] && lim["disk-files"].IsScalar()) {
							sl.disk_files = lim["disk-files"].as<size_t>();
						} else { // if not defined, load from default config
							sl.disk_files = default_config_->get_limits().disk_files;
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
				auto its = default_config_->get_headers().equal_range("hwgroup");
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
				std::shared_ptr<task_base> task =
						std::make_shared<external_task>(default_config_->get_worker_id(), id++, task_id,
														priority, fatal, task_depend, cmd, args,
														sandbox_name, limits, logger_);
				unconnected_tasks.insert(std::make_pair(task_id, task));
				eff_indegree.insert(std::make_pair(task_id, 0));

			} else {

				// //////////////// //
				// internal command //
				// //////////////// //

				std::shared_ptr<task_base> task;

				if (cmd == "cp") {
					task = std::make_shared<cp_task>(id++, task_id, priority, fatal, cmd, args, task_depend);
				} else if (cmd == "mkdir") {
					task = std::make_shared<mkdir_task>(id++, task_id, priority, fatal, cmd, args, task_depend);
				} else if (cmd == "rename") {
					task = std::make_shared<rename_task>(id++, task_id, priority, fatal, cmd, args, task_depend);
				} else if (cmd == "rm") {
					task = std::make_shared<rm_task>(id++, task_id, priority, fatal, cmd, args, task_depend);
				} else if (cmd == "archivate") {
					task = std::make_shared<archivate_task>(id++, task_id, priority, fatal, cmd, args, task_depend);
				} else if (cmd == "extract") {
					task = std::make_shared<extract_task>(id++, task_id, priority, fatal, cmd, args, task_depend);
				} else if (cmd == "fetch") {
					task = std::make_shared<fetch_task>(id++, task_id, priority, fatal, cmd, args, task_depend, fileman_);
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
					throw job_exception("Non existing task-id " + depend.at(i) + " in dependency list");
				}
			}
		}


		// all should be done now... just linear ordering is missing...
		try {
			helpers::topological_sort(root_task_, eff_indegree, task_queue_);
		} catch (helpers::top_sort_exception &e) {
			throw job_exception(e.what());
		}

	} catch (YAML::Exception &e) {
		throw job_exception("Exception in yaml-cpp: " + std::string(e.what()));
	}

	return;
}

void job::prepare_job()
{
	return;
}

void job::cleanup_job()
{
	// destroy all files in working directory
	// -> job_evaluator will handle this for us...

	return;
}

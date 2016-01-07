#include "job.h"

class task_compare {
public:
	/**
	 * Greater than operator on task_base objects.
	 * @param a
	 * @param b
	 * @return
	 */
	bool operator()(std::shared_ptr<task_base> a, std::shared_ptr<task_base> b) {
		if (a->get_priority() > b->get_priority()) {
			return true;
		} else if (a->get_priority() == b->get_priority() && a->get_id() > b->get_id()) {
			return true;
		}

		return false;
	}
};

job::job(const YAML::Node &job_config, boost::filesystem::path source_path,
		 std::shared_ptr<spdlog::logger> logger,
		 std::shared_ptr<worker_config> default_config,
		 std::shared_ptr<file_manager_base> fileman)
	: source_path_(source_path), fileman_(fileman), root_task_(nullptr),
	  logger_(logger), default_config_(default_config)
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

	// and... build and prepare job for evaluation
	build_job(job_config);
	prepare_job();
}

job::~job()
{
	cleanup_job();
}

void job::run()
{
	// simply run all tasks in given topological order
	for (auto &i : task_queue_) {
		i->run();
	}

	return;
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
			job_id_ = submiss["job-id"].as<size_t>();
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
					} else { throw job_exception(); }

					if (ctask["cmd"]["args"] && ctask["cmd"]["args"].IsSequence()) {
						args = ctask["cmd"]["args"].as<std::vector<std::string>>();
					} else { throw job_exception("Command arguments is not defined properly"); }
				} else { throw job_exception("Command in task is not a map"); }
			} else { throw job_exception("Configuration of one task has missing cmd"); }
			if (ctask["log"] && ctask["log"].IsScalar()) {
				log = ctask["log"].as<std::string>();
			} // can be omitted... no throw

			// load dependencies and check if defined tasks exists
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
						} // can be omitted... no throw
						if (lim["wall-time"] && lim["wall-time"].IsScalar()) {
							sl.wall_time = lim["wall-time"].as<float>();
						} // can be omitted... no throw
						if (lim["extra-time"] && lim["extra-time"].IsScalar()) {
							sl.extra_time = lim["extra-time"].as<float>();
						} // can be omitted... no throw
						if (lim["stack-size"] && lim["stack-size"].IsScalar()) {
							sl.stack_size = lim["stack-size"].as<size_t>();
						} // can be omitted... no throw
						if (lim["memory"] && lim["memory"].IsScalar()) {
							sl.memory_usage = lim["memory"].as<size_t>();
						} // can be omitted... no throw
						if (lim["parallel"] && lim["parallel"].IsScalar()) {
							lim["parallel"].as<bool>(); // TODO not defined properly
						} // can be omitted... no throw
						if (lim["disk-blocks"] && lim["disk-blocks"].IsScalar()) {
							sl.disk_blocks = lim["disk-blocks"].as<size_t>();
						} // can be omitted... no throw
						if (lim["disk-inodes"] && lim["disk-inodes"].IsScalar()) {
							sl.disk_inodes = lim["disk-inodes"].as<size_t>();
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
				//limits = hwgroups.at(default_config_->get_hwgroup()); // TODO
				limits.std_input = std_input;
				limits.std_output = std_output;
				limits.std_error = std_error;
				std::shared_ptr<task_base> task =
						std::make_shared<external_task>(id++, task_id, priority, fatal, log, task_depend,
														cmd, args, sandbox_name, limits);
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
				}  else if (cmd == "rm") {
					task = std::make_shared<rm_task>(id++, task_id, priority, fatal, cmd, args, log, task_depend);
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
				} catch (std::out_of_range &ex) {
					throw job_exception("Non existing task-id in dependency list");
				}
			}
		}


		// all should be done now... just linear ordering is missing...
		topological_sort(root_task_, eff_indegree, task_queue_);

	} catch (YAML::Exception &e) {
		throw job_exception("Exception in yaml-cpp: " + std::string(e.what()));
	}

	return;
}

void job::topological_sort(std::shared_ptr<task_base> root,
						   std::map<std::string, size_t> &effective_indegree,
						   std::vector<std::shared_ptr<task_base>> &result)
{
	// clean queue of tasks if there are any elements
	result.clear();

	std::priority_queue<std::shared_ptr<task_base>, std::vector<std::shared_ptr<task_base>>, task_compare> prior_queue;
	std::set<std::string> passed; // store tasks that were visited and queued

	prior_queue.push(root);
	passed.insert(root->get_task_id());

	while (!prior_queue.empty()) {
		auto top = prior_queue.top();
		prior_queue.pop();

		result.push_back(top);

		auto deps = top->get_children();
		for (auto &dep : deps) {
			if (effective_indegree.at(dep->get_task_id()) > 0) {
				size_t tmp = --effective_indegree.at(dep->get_task_id());

				if (tmp == 0) {
					prior_queue.push(dep);
					passed.insert(dep->get_task_id());
				}
			} else {
				throw job_exception("Cycle in tasks dependencies detected");
			}
		}
	}

	// In case that we do not walk through whole tree.
	// Usually it means that there are cycles in tree.
	if (passed.size() != effective_indegree.size()) {
		throw job_exception("Cycle in tasks dependencies detected");
	}

	return;
}

void job::prepare_job()
{
	// prepare file manager
	fileman_->set_params(fileman_hostname_, fileman_username_, fileman_passwd_);

	// prepare working directory (maybe not necessary)

	return;
}

void job::cleanup_job()
{
	// destroy all files in working directory

	return;
}

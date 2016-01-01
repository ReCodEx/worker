#include "job.h"

job::job(const YAML::Node &job_config, boost::filesystem::path source_path,
		 std::shared_ptr<spdlog::logger> logger,
		 std::shared_ptr<worker_config> default_config,
		 std::shared_ptr<file_manager_base> fileman)
	: source_path_(source_path), root_task_(nullptr),
	  logger_(logger), default_config_(default_config), fileman_(fileman)
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
		throw job_exception("Source code didrectory is empty");
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
	return;
}

void job::build_job(const YAML::Node &conf)
{
	// initial checkouts
	if (!conf.IsDefined()) {
		throw job_exception("Job config file was empty");
	} else if (!conf.IsMap()) {
		throw job_exception("Bad format of job configuration");
	} else if (conf["tasks"]) {
		throw job_exception("Item tasks was not given in job configuration");
	} else if (conf["submission"]) {
		throw job_exception("Item submission was not given in job configuration");
	} else if (!conf["tasks"].IsSequence()) {
		throw job_exception("Item tasks in job configuration is not sequence");
	} else if (!conf["submission"].IsMap()) {
		throw job_exception("Item submission in job configuration is not map");
	}


	// get information about submission
	if (conf["submission"]["job-id"]) {
		job_id_ = conf["submission"]["job-id"].as<size_t>();
	} else { throw job_exception("Submission.job-id item not loaded properly"); }
	if (conf["submission"]["language"]) {
		language_ = conf["submission"]["language"].as<std::string>();
	} else { throw job_exception("Submission.language item not loaded properly"); }
	if (conf["submission"]["file-collector"]) {
		if (conf["submission"]["file-collector"].IsMap()) {
			if (conf["submission"]["file-collector"]["hostname"]) {
				fileman_hostname_ = conf["submission"]["file-collector"]["hostname"].as<std::string>();
			} else { throw job_exception("Submission.file-collector.hostname item not loaded properly"); }
			if (conf["submission"]["file-collector"]["port"]) {
				fileman_port_ = conf["submission"]["file-collector"]["port"].as<std::string>();
			} else { throw job_exception("Submission.file-collector.port item not loaded properly"); }
			if (conf["submission"]["file-collector"]["username"]) {
				fileman_username_ = conf["submission"]["file-collector"]["username"].as<std::string>();
			} else { throw job_exception("Submission.file-collector.username item not loaded properly"); }
			if (conf["submission"]["file-collector"]["password"]) {
				fileman_passwd_ = conf["submission"]["file-collector"]["password"].as<std::string>();
			} else { throw job_exception("Submission.file-collector.password item not loaded properly"); }
		} else {
			throw job_exception("Item submission.file-collector is not map");
		}
	} else { throw job_exception("Submission item not loaded properly"); }


	// create fake task, which is logical root of evaluation
	root_task_ = std::make_shared<fake_task>();


	// construct all tasks with their ids and check if they have all datas, but do not connect them
	std::map<std::string, std::shared_ptr<task_base>> unconnected_tasks;
	for (size_t i = 0; i < conf["tasks"].size(); ++i) {
		std::string task_id;
		size_t priority;
		bool fatal;
		std::string cmd;
		std::string log;
		std::vector<std::string> task_depend;

		if (conf["tasks"][i]["task-id"]) {
			task_id = conf["tasks"][i]["task-id"].as<std::string>();
		} else { throw job_exception("Configuration of one task has missing task-id"); }
		if (conf["tasks"][i]["priority"]) {
			priority = conf["tasks"][i]["priority"].as<size_t>();
		} else { throw job_exception("Configuration of one task has missing priority"); }
		if (conf["tasks"][i]["fatal"]) {
			fatal = conf["tasks"][i]["fatal"].as<bool>();
		} else { throw job_exception("Configuration of one task has missing fatal-failure"); }
		if (conf["tasks"][i]["cmd"]) {
			cmd = conf["tasks"][i]["cmd"].as<std::string>();
		} else { throw job_exception("Configuration of one task has missing cmd"); }
		if (conf["tasks"][i]["log"]) {
			log = conf["tasks"][i]["log"].as<std::string>();
		} // can be omitted... no throw

		// load dependencies and check if defined tasks exists
		if (conf["tasks"][i]["dependencies"] && conf["tasks"][i]["dependencies"].IsSequence()) {
			task_depend = conf["tasks"][i]["dependencies"].as<std::vector<std::string>>();
		}

		// distinguish internal/external command and construct suitable object
		if (conf["tasks"][i]["sandbox"]) { // external command

			std::string std_input;
			std::string std_output;
			std::string std_error;
			std::string sandbox_name;
			std::map<std::string, sandbox_limits> hwgroups;
			sandbox_limits limits;

			if (conf["tasks"][i]["sandbox"]["name"]) {
				sandbox_name = conf["tasks"][i]["sandbox"]["name"].as<std::string>();
			} else { throw job_exception("Name of sandbox not given"); }

			if (conf["tasks"][i]["stdin"]) {
				std_input = conf["tasks"][i]["stdin"].as<std::string>();
			} // can be ommited... no throw
			if (conf["tasks"][i]["stdout"]) {
				std_output = conf["tasks"][i]["stdout"].as<std::string>();
			} // can be ommited... no throw
			if (conf["tasks"][i]["stderr"]) {
				std_error = conf["tasks"][i]["stderr"].as<std::string>();
			} // can be ommited... no throw

			// load limits... if they are supplied
			if (conf["tasks"][i]["sandbox"]["limits"]) {
				if (!conf["tasks"][i]["sandbox"]["limits"].IsSequence()) {
					throw job_exception("Sandbox limits are not sequence");
				}

				for (auto &lim : conf["tasks"][i]["sandbox"]["limits"]) {
					sandbox_limits sl;
					std::string hwgroup;

					if (lim["hw-groups-id"]) {
						hwgroup = lim["hw-groups-id"].as<std::string>();
					} else { throw job_exception("Hwgroup ID not defined in sandbox limits"); }

					if (lim["time"]) {
						sl.cpu_time = lim["time"].as<size_t>();
					} // can be omitted... no throw
					if (lim["wall-time"]) {
						sl.wall_time = lim["wall-time"].as<size_t>();
					} // can be omitted... no throw
					if (lim["extra-time"]) {
						sl.extra_time = lim["extra-time"].as<size_t>();
					} // can be omitted... no throw
					if (lim["stack-size"]) {
						sl.stack_size = lim["stack-size"].as<size_t>();
					} // can be omitted... no throw
					if (lim["memory"]) {
						sl.memory_usage = lim["memory"].as<size_t>();
					} // can be omitted... no throw
					if (lim["parallel"]) {
						lim["parallel"].as<bool>(); // TODO not defined properly
					} // can be omitted... no throw
					if (lim["disk-blocks"]) {
						sl.disk_blocks = lim["disk-blocks"].as<size_t>();
					} // can be omitted... no throw
					if (lim["disk-inodes"]) {
						sl.disk_inodes = lim["disk-inodes"].as<size_t>();
					} // can be omitted... no throw

					if (lim["environ-variable"] && lim["environ-variable"].IsMap()) {
						for (auto &var : lim["environ-variable"]) {
							sl.environ_vars.insert(std::make_pair(var.as<std::string>(), var.as<std::string>())); // TODO not loaded properly
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
					std::make_shared<external_task>(task_id, priority, fatal, log, task_depend,
													cmd, std::vector<std::string>(), sandbox_name, limits); // TODO args
			unconnected_tasks.insert(std::make_pair(task_id, task));

		} else {// internal command
			// TODO
		}
	}


	// constructed tasks in map have to have tree structure, so... make it and connect them
	for (auto &elem : unconnected_tasks) {
		const std::vector<std::string> &depend = elem.second->get_dependencies();

		// connect all suitable task underneath root
		if (depend.size() == 0) {
			root_task_->add_children(elem.second);
			elem.second->add_parent(root_task_);
		}

		for (size_t i = 0; i < depend.size(); ++i) {
			try {
				auto ptr = unconnected_tasks.at(depend.at(i));
				ptr->add_children(elem.second);
				elem.second->add_parent(ptr);
			} catch (std::out_of_range ex) {
				throw job_exception("Non existing task-id in dependency list");
			}
		}
	}


	// all should be done now... just linear ordering is missing...
	// TODO linear ordering

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

#include "config.h"


std::shared_ptr<job_metadata> helpers::build_job_metadata(const YAML::Node &conf)
{
	std::shared_ptr<job_metadata> job_meta = std::make_shared<job_metadata>();

	try {
		// initial checkouts
		if (!conf.IsDefined()) {
			throw config_exception("Job config file was empty");
		} else if (!conf.IsMap()) {
			throw config_exception("Job configuration is not a map");
		} else if (!conf["tasks"]) {
			throw config_exception("Item tasks was not given in job configuration");
		} else if (!conf["submission"]) {
			throw config_exception("Item submission was not given in job configuration");
		} else if (!conf["tasks"].IsSequence()) {
			throw config_exception("Item tasks in job configuration is not sequence");
		} else if (!conf["submission"].IsMap()) {
			throw config_exception("Item submission in job configuration is not map");
		}


		// get information about this submission
		auto submiss = conf["submission"];
		if (submiss["job-id"] && submiss["job-id"].IsScalar()) {
			job_meta->job_id = submiss["job-id"].as<std::string>();
		} else { throw config_exception("Submission.job-id item not loaded properly"); }
		if (submiss["language"] && submiss["language"].IsScalar()) {
			job_meta->language = submiss["language"].as<std::string>();
		} else { throw config_exception("Submission.language item not loaded properly"); }
		if (submiss["file-collector"] && submiss["file-collector"].IsScalar()) {
			job_meta->file_server_url = submiss["file-collector"].as<std::string>();
		} else { throw config_exception("Submission.file-collector item not loaded properly"); }


		// load datas for tasks and save them
		for (auto &ctask : conf["tasks"]) {
			std::shared_ptr<task_metadata> task_meta = std::make_shared<task_metadata>();

			if (ctask["task-id"] && ctask["task-id"].IsScalar()) {
				task_meta->task_id = ctask["task-id"].as<std::string>();
			} else { throw config_exception("Configuration task has missing task-id"); }
			if (ctask["priority"] && ctask["priority"].IsScalar()) {
				task_meta->priority = ctask["priority"].as<size_t>();
			} else { throw config_exception("Configuration task has missing priority"); }
			if (ctask["fatal-failure"] && ctask["fatal-failure"].IsScalar()) {
				task_meta->fatal_failure = ctask["fatal-failure"].as<bool>();
			} else { throw config_exception("Configuration task has missing fatal-failure"); }
			if (ctask["cmd"]) {
				if (ctask["cmd"].IsMap()) {
					if (ctask["cmd"]["bin"] && ctask["cmd"]["bin"].IsScalar()) {
						task_meta->binary = ctask["cmd"]["bin"].as<std::string>();
					} else { throw config_exception("Runnable binary for task not given"); }

					if (ctask["cmd"]["args"] && ctask["cmd"]["args"].IsSequence()) {
						task_meta->cmd_args = ctask["cmd"]["args"].as<std::vector<std::string>>();
					} // can be omitted... no throw
				} else { throw config_exception("Command in task is not a map"); }
			} else { throw config_exception("Configuration of one task has missing cmd"); }

			// load dependencies
			if (ctask["dependencies"] && ctask["dependencies"].IsSequence()) {
				task_meta->dependencies = ctask["dependencies"].as<std::vector<std::string>>();
			}

			if (ctask["stdin"] && ctask["stdin"].IsScalar()) {
				task_meta->std_input = ctask["stdin"].as<std::string>();
			} // can be ommited... no throw
			if (ctask["stdout"] && ctask["stdout"].IsScalar()) {
				task_meta->std_output = ctask["stdout"].as<std::string>();
			} // can be ommited... no throw
			if (ctask["stderr"] && ctask["stderr"].IsScalar()) {
				task_meta->std_error = ctask["stderr"].as<std::string>();
			} // can be ommited... no throw

			// distinguish internal/external command and construct suitable object
			if (ctask["sandbox"]) {

				// //////////////// //
				// external command //
				// //////////////// //

				std::shared_ptr<sandbox_config> sandbox = std::make_shared<sandbox_config>();

				if (ctask["sandbox"]["name"] && ctask["sandbox"]["name"].IsScalar()) {
					sandbox->name = ctask["sandbox"]["name"].as<std::string>();
				} else { throw config_exception("Name of sandbox not given"); }

				// load limits... if they are supplied
				if (ctask["sandbox"]["limits"]) {
					if (!ctask["sandbox"]["limits"].IsSequence()) {
						throw config_exception("Sandbox limits are not sequence");
					}

					for (auto &lim : ctask["sandbox"]["limits"]) {
						std::shared_ptr<sandbox_limits> sl = std::make_shared<sandbox_limits>();
						std::string hwgroup;

						if (lim["hw-group-id"] && lim["hw-group-id"].IsScalar()) {
							hwgroup = lim["hw-group-id"].as<std::string>();
						} else { throw config_exception("Hwgroup ID not defined in sandbox limits"); }

						if (lim["time"] && lim["time"].IsScalar()) {
							sl->cpu_time = lim["time"].as<float>();
						} else {
							sl->cpu_time = FLT_MAX; // set undefined value (max float)
						}
						if (lim["wall-time"] && lim["wall-time"].IsScalar()) {
							sl->wall_time = lim["wall-time"].as<float>();
						} else {
							sl->wall_time = FLT_MAX; // set undefined value (max float)
						}
						if (lim["extra-time"] && lim["extra-time"].IsScalar()) {
							sl->extra_time = lim["extra-time"].as<float>();
						} else {
							sl->extra_time = FLT_MAX; // set undefined value (max float)
						}
						if (lim["stack-size"] && lim["stack-size"].IsScalar()) {
							sl->stack_size = lim["stack-size"].as<size_t>();
						} else {
							sl->stack_size = SIZE_MAX; // set undefined value (max size_t)
						}
						if (lim["memory"] && lim["memory"].IsScalar()) {
							sl->memory_usage = lim["memory"].as<size_t>();
						} else {
							sl->memory_usage = SIZE_MAX; // set undefined value (max size_t)
						}
						if (lim["parallel"] && lim["parallel"].IsScalar()) { // TODO not defined properly
							sl->processes = lim["parallel"].as<size_t>();
						}
						if (lim["disk-blocks"] && lim["disk-blocks"].IsScalar()) {
							sl->disk_blocks = lim["disk-blocks"].as<size_t>();
						} else {
							sl->disk_blocks = SIZE_MAX; // set undefined value (max size_t)
						}
						if (lim["disk-inodes"] && lim["disk-inodes"].IsScalar()) {
							sl->disk_inodes = lim["disk-inodes"].as<size_t>();
						} else {
							sl->disk_inodes = SIZE_MAX; // set undefined value (max size_t)
						}
						if (lim["chdir"] && lim["chdir"].IsScalar()) {
							sl->chdir = lim["chdir"].as<std::string>();
						}

						if (lim["bound-directories"] && lim["bound-directories"].IsMap()) {
							sl->bound_dirs = lim["bound-directories"].as<std::map<std::string, std::string>>();
						} // can be omitted... no throw

						if (lim["environ-variable"] && lim["environ-variable"].IsMap()) {
							for (auto &var : lim["environ-variable"]) {
								sl->environ_vars.insert(std::make_pair(var.first.as<std::string>(), var.second.as<std::string>()));
							}
						}

						sandbox->limits.insert(std::make_pair(hwgroup, sl));
					}
				}

				task_meta->sandbox = sandbox;
			} else {

				// //////////////// //
				// internal command //
				// //////////////// //

				// nothing to do here right now
			}

			// add task to job_meta
			job_meta->tasks.push_back(task_meta);
		}

	}  catch (YAML::Exception &e) {
		throw config_exception("Exception in yaml-cpp: " + std::string(e.what()));
	}

	return job_meta;
}

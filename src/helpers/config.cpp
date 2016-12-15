#include <algorithm>
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
		} else {
			throw config_exception("Submission.job-id item not loaded properly");
		}
		if (submiss["file-collector"] && submiss["file-collector"].IsScalar()) {
			job_meta->file_server_url = submiss["file-collector"].as<std::string>();
		} else {
			throw config_exception("Submission.file-collector item not loaded properly");
		}
		if (submiss["log"] && submiss["log"].IsScalar()) {
			job_meta->log = submiss["log"].as<bool>();
		} // can be omitted... no throw
		if (submiss["hw-groups"] && submiss["hw-groups"].IsSequence()) {
			job_meta->hwgroups = submiss["hw-groups"].as<std::vector<std::string>>();
		} else {
			throw config_exception("Submission.hw-groups item not loaded properly");
		}


		// load datas for tasks and save them
		for (auto &ctask : conf["tasks"]) {
			std::shared_ptr<task_metadata> task_meta = std::make_shared<task_metadata>();

			if (ctask["task-id"] && ctask["task-id"].IsScalar()) {
				task_meta->task_id = ctask["task-id"].as<std::string>();
			} else {
				throw config_exception("Configuration task has missing task-id");
			}
			if (ctask["priority"] && ctask["priority"].IsScalar()) {
				task_meta->priority = ctask["priority"].as<size_t>();
			} else {
				task_meta->priority = 1; // default value
			}
			if (ctask["fatal-failure"] && ctask["fatal-failure"].IsScalar()) {
				task_meta->fatal_failure = ctask["fatal-failure"].as<bool>();
			} else {
				task_meta->fatal_failure = false; // default value
			}
			if (ctask["cmd"]) {
				if (ctask["cmd"].IsMap()) {
					if (ctask["cmd"]["bin"] && ctask["cmd"]["bin"].IsScalar()) {
						task_meta->binary = ctask["cmd"]["bin"].as<std::string>();
					} else {
						throw config_exception("Runnable binary for task not given");
					}

					if (ctask["cmd"]["args"] && ctask["cmd"]["args"].IsSequence()) {
						task_meta->cmd_args = ctask["cmd"]["args"].as<std::vector<std::string>>();
					} // can be omitted... no throw
				} else {
					throw config_exception("Command in task is not a map");
				}
			} else {
				throw config_exception("Configuration of one task has missing cmd");
			}

			// load dependencies
			if (ctask["dependencies"] && ctask["dependencies"].IsSequence()) {
				task_meta->dependencies = ctask["dependencies"].as<std::vector<std::string>>();
			}

			// load task type
			if (ctask["type"] && ctask["type"].IsScalar()) {
				task_meta->type = helpers::get_task_type(ctask["type"].as<std::string>());
			}

			// distinguish internal/external command and construct suitable object
			if (ctask["sandbox"]) {

				// //////////////// //
				// external command //
				// //////////////// //

				std::string std_input;
				std::string std_output;
				std::string std_error;
				std::shared_ptr<sandbox_config> sandbox = std::make_shared<sandbox_config>();

				if (ctask["sandbox"]["name"] && ctask["sandbox"]["name"].IsScalar()) {
					sandbox->name = ctask["sandbox"]["name"].as<std::string>();
				} else {
					throw config_exception("Name of sandbox not given");
				}

				if (ctask["sandbox"]["stdin"] && ctask["sandbox"]["stdin"].IsScalar()) {
					std_input = ctask["sandbox"]["stdin"].as<std::string>();
				} // can be ommited... no throw
				if (ctask["sandbox"]["stdout"] && ctask["sandbox"]["stdout"].IsScalar()) {
					std_output = ctask["sandbox"]["stdout"].as<std::string>();
				} // can be ommited... no throw
				if (ctask["sandbox"]["stderr"] && ctask["sandbox"]["stderr"].IsScalar()) {
					std_error = ctask["sandbox"]["stderr"].as<std::string>();
				} // can be ommited... no throw

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
						} else {
							throw config_exception("Hwgroup ID not defined in sandbox limits");
						}

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
						} else {
							sl->processes = SIZE_MAX; // set undefined value (max size_t)
						}
						if (lim["disk-size"] && lim["disk-size"].IsScalar()) {
							sl->disk_size = lim["disk-size"].as<size_t>();
						} else {
							sl->disk_size = SIZE_MAX; // set undefined value (max size_t)
						}
						if (lim["disk-files"] && lim["disk-files"].IsScalar()) {
							sl->disk_files = lim["disk-files"].as<size_t>();
						} else {
							sl->disk_files = SIZE_MAX; // set undefined value (max size_t)
						}
						if (lim["chdir"] && lim["chdir"].IsScalar()) {
							sl->chdir = lim["chdir"].as<std::string>();
						}

						sl->bound_dirs = helpers::get_bind_dirs(lim);

						if (lim["environ-variable"] && lim["environ-variable"].IsMap()) {
							for (auto &var : lim["environ-variable"]) {
								sl->environ_vars.push_back(
									std::make_pair(var.first.as<std::string>(), var.second.as<std::string>()));
							}
						}

						sl->std_input = std_input;
						sl->std_output = std_output;
						sl->std_error = std_error;

						sandbox->loaded_limits.insert(std::make_pair(hwgroup, sl));
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

	} catch (YAML::Exception &e) {
		throw config_exception("Exception in yaml-cpp: " + std::string(e.what()));
	}

	return job_meta;
}

task_type helpers::get_task_type(const std::string &type)
{
	std::string lower = type;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	if (lower == "evaluation") {
		return task_type::EVALUATION;
	} else if (lower == "execution") {
		return task_type::EXECUTION;
	} else if (lower == "initiation") {
		return task_type::INITIATION;
	}

	return task_type::INNER;
}

std::vector<std::tuple<std::string, std::string, sandbox_limits::dir_perm>> helpers::get_bind_dirs(
	const YAML::Node &lim)
{
	std::vector<std::tuple<std::string, std::string, sandbox_limits::dir_perm>> bound_dirs;

	if (lim["bound-directories"] && lim["bound-directories"].IsSequence()) {
		for (auto &dir : lim["bound-directories"]) {
			std::string src;
			std::string dst;
			sandbox_limits::dir_perm mode = sandbox_limits::dir_perm::RO;
			if (dir.IsMap()) {
				if (dir["src"] && dir["src"].IsScalar()) {
					src = dir["src"].as<std::string>();
				} else {
					throw config_exception("Item 'src' in 'bound-directories' not defined");
				}
				if (dir["dst"] && dir["dst"].IsScalar()) {
					dst = dir["dst"].as<std::string>();
				} else {
					throw config_exception("Item 'dst' in 'bound-directories' not defined");
				}
				if (dir["mode"] && dir["mode"].IsScalar()) {
					std::string str_mode = dir["mode"].as<std::string>();
					std::transform(str_mode.begin(), str_mode.end(), str_mode.begin(), ::toupper);

					if (str_mode.find("RW") != std::string::npos) {
						mode = static_cast<sandbox_limits::dir_perm>(mode | sandbox_limits::dir_perm::RW);
					}
					if (str_mode.find("NOEXEC") != std::string::npos) {
						mode = static_cast<sandbox_limits::dir_perm>(mode | sandbox_limits::dir_perm::NOEXEC);
					}
					if (str_mode.find("FS") != std::string::npos) {
						mode = static_cast<sandbox_limits::dir_perm>(mode | sandbox_limits::dir_perm::FS);
					}
					if (str_mode.find("MAYBE") != std::string::npos) {
						mode = static_cast<sandbox_limits::dir_perm>(mode | sandbox_limits::dir_perm::MAYBE);
					}
					if (str_mode.find("DEV") != std::string::npos) {
						mode = static_cast<sandbox_limits::dir_perm>(mode | sandbox_limits::dir_perm::DEV);
					}
				} // no throw... can be omitted
				bound_dirs.push_back(std::tuple<std::string, std::string, sandbox_limits::dir_perm>{src, dst, mode});
			}
		}
	} // can be omitted... no throw

	return bound_dirs;
}

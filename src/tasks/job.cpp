#include "job.h"

job::job(std::string submission_path,
		 std::shared_ptr<spdlog::logger> logger,
		 std::shared_ptr<worker_config> config,
		 std::shared_ptr<file_manager_base> fileman)
	: submission_path_(submission_path), root_task_(nullptr),
	  logger_(logger), default_config_(config), fileman_(fileman)
{
	load_config();
	build_job();
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

void job::load_config()
{
	using namespace boost::filesystem;
	path config_path(submission_path_);
	config_path /= "config.yml";
	if (!exists(config_path)) {
		throw job_exception("Job configuration not found");
	}

	try {
		config_ = YAML::LoadFile(config_path.string());
	} catch (YAML::Exception e) {
		throw job_exception("Job configuration not loaded correctly: " + std::string(e.what()));
	}

	return;
}

void job::build_job()
{
	// initial checkouts
	if (!config_.IsDefined()) {
		throw job_exception("Job config file was empty");
	} else if (!config_.IsMap()) {
		throw job_exception("Bad format of job configuration");
	} else if (config_["tasks"]) {
		throw job_exception("Item tasks was not given in job configuration");
	} else if (config_["submission"]) {
		throw job_exception("Item submission was not given in job configuration");
	} else if (!config_["tasks"].IsSequence()) {
		throw job_exception("Item tasks in job configuration is not sequence");
	} else if (!config_["submission"].IsMap()) {
		throw job_exception("Item submission in job configuration is not map");
	}


	// get information about submission
	if (config_["submission"]["job-id"]) {
		job_id_ = config_["submission"]["job-id"].as<size_t>();
	} else { throw job_exception("Submission item not loaded properly"); }
	if (config_["submission"]["language"]) {
		language_ = config_["submission"]["language"].as<std::string>();
	} else { throw job_exception("Submission item not loaded properly"); }
	if (config_["submission"]["file-collector"]) {
		if (config_["submission"]["file-collector"].IsMap()) {
			if (config_["submission"]["file-collector"]["hostname"]) {
				fileman_hostname_ = config_["submission"]["file-collector"]["hostname"].as<std::string>();
			} else { throw job_exception("Submission.file-collector item not loaded properly"); }
			if (config_["submission"]["file-collector"]["port"]) {
				fileman_port_ = config_["submission"]["file-collector"]["port"].as<std::string>();
			} else { throw job_exception("Submission.file-collector item not loaded properly"); }
			if (config_["submission"]["file-collector"]["username"]) {
				fileman_username_ = config_["submission"]["file-collector"]["username"].as<std::string>();
			} else { throw job_exception("Submission.file-collector item not loaded properly"); }
			if (config_["submission"]["file-collector"]["password"]) {
				fileman_passwd_ = config_["submission"]["file-collector"]["password"].as<std::string>();
			} else { throw job_exception("Submission.file-collector item not loaded properly"); }
		} else {
			throw job_exception("Item submission.file-collector is not map");
		}
	} else { throw job_exception("Submission item not loaded properly"); }


	// make set of task ids just to know what tasks we have
	std::set<std::string> dependencies;
	for (size_t i = 0; i < config_["tasks"].size(); ++i) {
		if (config_["tasks"][i]["task-id"]) {
			dependencies.insert(config_["tasks"][i]["task-id"].as<std::string>());
		} else {
			throw job_exception("One of the tasks in job configuration do not have task-id set");
		}
	}


	// create fake task, which is logical root of evaluation
	root_task_ = std::make_shared<fake_task>();


	// construct all tasks and check if they have all datas, but do not connect them
	std::map<std::string, std::shared_ptr<task_base>> unconnected_tasks;
	for (size_t i = 0; i < config_["tasks"].size(); ++i) {
		std::string task_id;
		size_t priority;
		bool fatal;
		std::string cmd;
		std::string log;
		std::vector<std::string> task_depend;

		if (config_["tasks"][i]["task-id"]) {
			task_id = config_["tasks"][i]["task-id"].as<std::string>();
		} else { throw job_exception("Configuration of one task is in bad format"); }
		if (config_["tasks"][i]["priority"]) {
			priority = config_["tasks"][i]["priority"].as<size_t>();
		} else { throw job_exception("Configuration of one task is in bad format"); }
		if (config_["tasks"][i]["fatal"]) {
			fatal = config_["tasks"][i]["fatal"].as<bool>();
		} else { throw job_exception("Configuration of one task is in bad format"); }
		if (config_["tasks"][i]["cmd"]) {
			cmd = config_["tasks"][i]["cmd"].as<std::string>();
		} else { throw job_exception("Configuration of one task is in bad format"); }
		if (config_["tasks"][i]["log"]) {
			log = config_["tasks"][i]["log"].as<std::string>();
		} else { throw job_exception("Configuration of one task is in bad format"); }
		if (config_["tasks"][i]["dependencies"]) {
			task_depend = config_["tasks"][i]["dependencies"].as<std::vector<std::string>>();
		} else { throw job_exception("Configuration of one task is in bad format"); }

		// distinguish internal/external command and construct suitable object
		if (config_["tasks"][i]["sandbox"]) {
			// external command
		} else {
			// internal command
		}
	}


	// constructed tasks in map have to have tree structure, so... make it and connect them
	for (auto &elem : unconnected_tasks) {
		const std::vector<std::string> &depend = elem.second->get_dependencies();

		for (size_t i = 0; i < depend.size(); ++i) {
			auto ptr = unconnected_tasks.at(depend.at(i));
			ptr->add_children(elem.second);
			elem.second->add_parent(ptr);
		}
	}


	// all should be done now... just linear ordering is missing...

	return;
}

void job::prepare_job()
{
	return;
}

void job::cleanup_job()
{
	return;
}

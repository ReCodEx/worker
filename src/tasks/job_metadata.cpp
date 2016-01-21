#include <map>
#include <yaml-cpp/yaml.h>

#include "job_metadata.h"
#include "job_exception.h"
#include "../helpers/topological_sort.h"

job_metadata::job_metadata(const YAML::Node &config, std::shared_ptr<const worker_config> worker_config)
{
	try {
		// initial checkouts
		if (!config.IsDefined()) {
			throw job_exception("Job config file was empty");
		} else if (!config.IsMap()) {
			throw job_exception("Job configuration is not a map");
		} else if (!config["tasks"]) {
			throw job_exception("Item tasks was not given in job configuration");
		} else if (!config["submission"]) {
			throw job_exception("Item submission was not given in job configuration");
		} else if (!config["tasks"].IsSequence()) {
			throw job_exception("Item tasks in job configuration is not sequence");
		} else if (!config["submission"].IsMap()) {
			throw job_exception("Item submission in job configuration is not map");
		}


		// get information about submission
		auto submiss = config["submission"];
		if (submiss["job-id"] && submiss["job-id"].IsScalar()) {
			job_id_ = submiss["job-id"].as<std::string>();
		} else { throw job_exception("Submission.job-id item not loaded properly"); }
		if (submiss["language"] && submiss["language"].IsScalar()) {
			language_ = submiss["language"].as<std::string>();
		} else { throw job_exception("Submission.language item not loaded properly"); }
		if (submiss["file-collector"] && submiss["file-collector"].IsScalar()) {
			file_server_url_ = submiss["file-collector"].as<std::string>();
		} else { throw job_exception("Submission.file-collector item not loaded properly"); }



	} catch (YAML::Exception &e) {
		throw job_exception("Exception in yaml-cpp: " + std::string(e.what()));
	}

}

std::string job_metadata::get_job_id()
{
	return job_id_;
}

std::string job_metadata::get_language()
{
	return language_;
}

const std::vector<std::shared_ptr<task_base>> job_metadata::get_tasks()
{
	return tasks_;
}

std::string job_metadata::get_file_server_url()
{
	return file_server_url_;
}

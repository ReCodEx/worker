#include "job_evaluator.h"
#include "../config/job_metadata.h"
#include "../fileman/fallback_file_manager.h"
#include "../fileman/prefixed_file_manager.h"
#include "../helpers/config.h"
#include "job_exception.h"

job_evaluator::job_evaluator(std::shared_ptr<spdlog::logger> logger,
	std::shared_ptr<worker_config> config,
	std::shared_ptr<file_manager_base> remote_fm,
	std::shared_ptr<file_manager_base> cache_fm,
	fs::path working_directory,
	std::shared_ptr<progress_callback_base> progr_callback)
	: working_directory_(working_directory), job_(nullptr), job_results_(), remote_fm_(remote_fm), cache_fm_(cache_fm),
	  logger_(logger), config_(config), progress_callback_(progr_callback)
{
	if (logger_ == nullptr) {
		logger_ = helpers::create_null_logger();
	}

	init_progress_callback();
}

job_evaluator::~job_evaluator()
{
}

void job_evaluator::init_progress_callback()
{
	if (progress_callback_ == nullptr) {
		progress_callback_ = std::make_shared<empty_progress_callback>();
	}
}

void job_evaluator::download_submission()
{
	logger_->info() << "Trying to download submission archive...";

	// create directory for downloaded archive
	fs::path archive_url = archive_url_;
	try {
		fs::create_directories(archive_path_);
	} catch (fs::filesystem_error &e) {
		throw job_exception(std::string("Cannot create downloading library: ") + e.what());
	}

	// download a file
	archive_name_ = archive_url.filename();
	remote_fm_->get_file(archive_url.string(), (archive_path_ / archive_name_).string());

	logger_->info() << "Submission archive downloaded succesfully.";
	progress_callback_->submission_downloaded(job_id_);
	return;
}

void job_evaluator::prepare_submission()
{
	logger_->info() << "Preparing submission for usage...";

	// decompress downloaded archive
	try {
		fs::create_directories(submission_path_);
		archivator::decompress((archive_path_ / archive_name_).string(), submission_path_.string());
	} catch (archive_exception &e) {
		throw job_exception("Downloaded submission cannot be decompressed: " + std::string(e.what()));
	}

	// copy source codes to source code folder
	try {
		// stem().stem() is removing the .tar.bz2 ending (maybe working with just .zip ending too)
		fs::path tmp_path = submission_path_ / archive_name_.stem().stem();
		// source_path_ is automaticaly created in copy_directory()
		helpers::copy_directory(tmp_path, source_path_);
		fs::permissions(source_path_, fs::add_perms | fs::group_write | fs::others_write);
	} catch (helpers::filesystem_exception &e) {
		throw job_exception("Error copying source files to source code path: " + std::string(e.what()));
	}

	try {
		fs::create_directories(results_path_);
	} catch (fs::filesystem_error &e) {
		throw job_exception("Result folder cannot be created: " + std::string(e.what()));
	}

	try {
		fs::create_directories(job_temp_dir_);
	} catch (fs::filesystem_error &e) {
		throw job_exception("Cannot create directories: " + std::string(e.what()));
	}


	logger_->info() << "Submission prepared.";
	return;
}

void job_evaluator::build_job()
{
	logger_->info() << "Building job...";

	// find job-config.yml to load configuration
	using namespace boost::filesystem;
	path config_path(source_path_);
	config_path /= "job-config.yml";
	if (!exists(config_path)) {
		throw job_exception("Job configuration not found");
	}

	// load configuration to object
	logger_->info() << "Loading job configuration from yaml...";
	YAML::Node conf;
	try {
		conf = YAML::LoadFile(config_path.string());
	} catch (YAML::Exception &e) {
		throw job_exception("Job configuration not loaded correctly: " + std::string(e.what()));
	}
	logger_->info() << "Yaml job configuration loaded properly.";

	// build job_metadata structure
	std::shared_ptr<job_metadata> job_meta = nullptr;
	try {
		job_meta = helpers::build_job_metadata(conf);
	} catch (helpers::config_exception &e) {
		throw job_exception("Job configuration loading problem: " + std::string(e.what()));
	}

	// construct manager which is used in task factory
	auto task_fileman = std::make_shared<fallback_file_manager>(
		cache_fm_, std::make_shared<prefixed_file_manager>(remote_fm_, job_meta->file_server_url + "/"));

	auto factory = std::make_shared<task_factory>(task_fileman);

	// ... and construct job itself
	job_ = std::make_shared<job>(
		job_meta, config_, job_temp_dir_, source_path_, results_path_, factory, progress_callback_);

	logger_->info() << "Job building done.";
	return;
}

void job_evaluator::run_job()
{
	try {
		logger_->info() << "Ready for evaluation...";
		job_results_ = job_->run();
		logger_->info() << "Job evaluated.";
	} catch (std::exception) {
		result_ = 1;
	} catch (...) {
		throw;
	}
	return;
}

void job_evaluator::init_submission_paths()
{
	source_path_ = working_directory_ / "eval" / std::to_string(config_->get_worker_id()) / job_id_;
	submission_path_ = working_directory_ / "submissions" / std::to_string(config_->get_worker_id()) / job_id_;
	archive_path_ = working_directory_ / "downloads" / std::to_string(config_->get_worker_id()) / job_id_;
	// set temporary directory for tasks in job
	job_temp_dir_ = working_directory_ / "temp" / std::to_string(config_->get_worker_id()) / job_id_;
	results_path_ = working_directory_ / "results" / std::to_string(config_->get_worker_id()) / job_id_;
}

void job_evaluator::cleanup_submission()
{
	// cleanup source code directory after job evaluation
	try {
		if (fs::exists(source_path_)) {
			logger_->info() << "Cleaning up source code directory...";
			fs::remove_all(source_path_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Source code directory not cleaned properly: " << e.what();
	}

	// delete submission decompressed directory
	try {
		if (fs::exists(submission_path_)) {
			logger_->info() << "Cleaning up decompressed submission directory...";
			fs::remove_all(submission_path_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Submission directory not cleaned properly: " << e.what();
	}

	// delete downloaded archive directory
	try {
		if (fs::exists(archive_path_)) {
			logger_->info() << "Cleaning up directory containing downloaded archive...";
			fs::remove_all(archive_path_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Archive directory not cleaned properly: " << e.what();
	}

	// delete temp directory
	try {
		if (fs::exists(job_temp_dir_)) {
			logger_->info() << "Cleaning up temp directory for tasks...";
			fs::remove_all(job_temp_dir_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Temp directory not cleaned properly: " << e.what();
	}

	// and finally delete created results directory
	try {
		if (fs::exists(results_path_)) {
			logger_->info() << "Cleaning up directory containing created results...";
			fs::remove_all(results_path_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Results directory not cleaned properly: " << e.what();
	}

	return;
}

void job_evaluator::cleanup_variables()
{
	try {
		archive_url_ = "";
		archive_name_ = "";
		archive_path_ = "";
		submission_path_ = "";
		source_path_ = "";
		results_path_ = "";
		result_url_ = "";

		job_id_ = "";
		job_ = nullptr;
		result_ = 0;
	} catch (std::exception &e) {
		logger_->warn() << "Error in deinicialization of evaluator: " << e.what();
	}
}

void job_evaluator::prepare_evaluator()
{
	init_submission_paths();
	cleanup_submission();
}

void job_evaluator::cleanup_evaluator()
{
	// cleanup_submission(); // TODO: just for debugging purposes
	cleanup_variables();
}

void job_evaluator::push_result()
{
	logger_->info() << "Trying to upload results of job...";

	// just checkout for errors
	if (job_ == nullptr) {
		logger_->warn() << "Pointer to job is null.";
		return;
	}

	// define path to result yaml file and archived result
	fs::path result_yaml = results_path_ / "result.yml";
	fs::path archive_path = results_path_ / "result.zip";

	logger_->info() << "Building yaml results file...";
	// build yaml tree
	YAML::Node res;
	res["job-id"] = job_id_;
	for (auto &i : job_results_) {
		YAML::Node node;
		node["task-id"] = i.first;
		node["status"] = i.second->failed ? "FAILED" : "OK";
		if (!i.second->error_message.empty()) {
			node["error_message"] = i.second->error_message;
		}

		auto &sandbox = i.second->sandbox_status;
		if (sandbox != nullptr) {
			YAML::Node subnode;
			subnode["exitcode"] = sandbox->exitcode;
			subnode["time"] = sandbox->time;
			subnode["wall-time"] = sandbox->wall_time;
			subnode["memory"] = sandbox->memory;
			subnode["max-rss"] = sandbox->max_rss;

			switch (sandbox->status) {
			case isolate_status::OK: subnode["status"] = "OK"; break;
			case isolate_status::RE: subnode["status"] = "RE"; break;
			case isolate_status::SG: subnode["status"] = "SG"; break;
			case isolate_status::TO: subnode["status"] = "TO"; break;
			case isolate_status::XX: subnode["status"] = "XX"; break;
			}

			subnode["exitsig"] = sandbox->exitsig;
			subnode["killed"] = sandbox->killed;
			subnode["message"] = sandbox->message;

			node["sandbox_results"] = subnode;
		}

		res["results"].push_back(node);
	}

	// open output stream and write constructed yaml
	std::ofstream out(result_yaml.string());
	out << res;
	out.close();
	logger_->info() << "Yaml result file written succesfully.";

	// compress given result.yml file
	logger_->info() << "Compression of results file...";
	try {
		archivator::compress(results_path_.string(), archive_path.string());
	} catch (archive_exception &e) {
		logger_->warn() << "Results file not archived properly: " << e.what();
		return;
	}
	logger_->info() << "Compression done.";

	// send archived result to file server
	remote_fm_->put_file(archive_path.string(), result_url_);

	logger_->info() << "Job results uploaded succesfully.";
	progress_callback_->job_results_uploaded(job_id_);
	return;
}

eval_response job_evaluator::evaluate(eval_request request)
{
	logger_->info() << "Request for job evaluation arrived to worker";
	logger_->info() << "Job ID of incoming job is: " + request.job_id;

	// set all needed variables for evaluation
	job_id_ = request.job_id;
	archive_url_ = request.job_url;
	result_url_ = request.result_url;
	std::cerr << "Job ID: " << job_id_ << std::endl; // TODO: just for debugging purposes
	std::cerr << "Archive url: " << archive_url_ << std::endl; // TODO: just for debugging purposes
	std::cerr << "Result url: " << result_url_ << std::endl; // TODO: just for debugging purposes

	// prepare result of response which will be sent to broker
	std::string response_result = "OK";

	prepare_evaluator();
	try {
		download_submission();
		prepare_submission();
		build_job();
		run_job();
		push_result();
	} catch (std::exception &e) {
		logger_->warn() << "Job evaluator encountered error: " << e.what();
		response_result = "ERR";
	}
	cleanup_evaluator();

	logger_->info() << "Job (" + request.job_id + ") ended.";

	return eval_response(request.job_id, response_result);
}

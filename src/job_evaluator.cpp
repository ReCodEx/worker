#include "job_evaluator.h"

job_evaluator::job_evaluator(std::shared_ptr<spdlog::logger> logger,
							 std::shared_ptr<worker_config> config,
							 std::shared_ptr<file_manager_base> fileman,
							 std::shared_ptr<file_manager_base> submission_fileman)
	: archive_url_(), archive_local_(), job_(nullptr),
	  fileman_(fileman), submission_fileman_(submission_fileman),
	  logger_(logger), config_(config)
{}

job_evaluator::~job_evaluator()
{}

void job_evaluator::download_submission()
{
	logger_->info() << "Trying to download submission archive...";

	// initialize all paths
	fs::path archive_url = archive_url_;
	archive_local_ = fs::temp_directory_path() / "isoeval" / "tmp" / "download" /
			config_->get_worker_id() / job_id_;
	fs::create_directories(archive_local_);

	// download a file
	submission_fileman_->set_params("", "", ""); // TODO resolve filemanager problems
	submission_fileman_->get_file(archive_url.string(), archive_local_.string());
	archive_local_ /= archive_url.filename();

	logger_->info() << "Submission archive donwloaded succesfully.";
	return;
}

void job_evaluator::prepare_submission()
{
	logger_->info() << "Preparing submission for usage...";

	// initialize all paths
	submission_path_ = fs::temp_directory_path() / "isoeval" / "tmp" / "submission" /
			config_->get_worker_id() / job_id_;
	source_path_ = fs::temp_directory_path() / "isoeval" / config_->get_worker_id() / job_id_;
	fs::create_directories(submission_path_);
	fs::create_directories(source_path_);

	// decompress downloaded archive
	try {
		archivator::decompress(archive_local_.string(), submission_path_.string());
	} catch (archive_exception &e) {
		throw job_exception("Downloaded submission cannot be decompressed.");
	}

	// copy source codes to source code folder
	try {
		if (fs::is_directory(submission_path_)) {
			fs::recursive_directory_iterator endit;
			for (fs::recursive_directory_iterator it(submission_path_); it != endit; ++it) {
				if (fs::is_regular_file(*it)) {
					fs::copy(*it, source_path_ / it->path().filename());
				}
			}
		}
	} catch (fs::filesystem_error &e) {
		throw job_exception("Source codes were not copied to source code direcotry.");
	}

	logger_->info() << "Submission prepared.";
	return;
}

void job_evaluator::build_job()
{
	logger_->info() << "Building job...";
	using namespace boost::filesystem;
	path config_path(source_path_);
	config_path /= "job-config.yml";
	if (!exists(config_path)) {
		throw job_exception("Job configuration not found");
	}

	logger_->info() << "Loading job configuration from yaml...";
	YAML::Node conf;
	try {
		conf = YAML::LoadFile(config_path.string());
	} catch (YAML::Exception &e) {
		throw job_exception("Job configuration not loaded correctly: " + std::string(e.what()));
	}
	logger_->info() << "Yaml job configuration loaded properly.";

	job_ = std::make_shared<job>(conf, source_path_, config_, fileman_);

	logger_->info() << "Job building done.";
	return;
}

void job_evaluator::run_job()
{
	try {
		logger_->info() << "Ready for evaluation...";
		job_->run();
		logger_->info() << "Job evaluated.";
	} catch (std::exception) {
		result_ = 1;
	} catch (...) {
		throw;
	}
	return;
}

void job_evaluator::cleanup_submission()
{
	// if job did not delete working dir, do it
	try {
		logger_->info() << "Cleaning up source code directory...";
		if (fs::exists(source_path_)) {
			fs::remove_all(source_path_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Source code directory not cleaned properly: " << e.what();
	}

	// delete submission decompressed directory
	try {
		logger_->info() << "Cleaning up decompressed submission directory...";
		if (fs::exists(submission_path_)) {
			fs::remove_all(submission_path_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Submission directory not cleaned properly: " << e.what();
	}

	// delete downloaded archive directory
	try {
		logger_->info() << "Cleaning up directory containing downloaded archive...";
		if (fs::exists(archive_local_)) {
			fs::remove_all(archive_local_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Archive directory not cleaned properly: " << e.what();
	}

	// and finally delete created results directory
	try {
		logger_->info() << "Cleaning up directory containing created results...";
		if (fs::exists(results_path_)) {
			fs::remove_all(results_path_);
		}
	} catch (fs::filesystem_error &e) {
		logger_->warn() << "Results directory not cleaned properly: " << e.what();
	}

	return;
}

void job_evaluator::cleanup_evaluator()
{
	cleanup_submission();

	try {
		archive_url_ = "";
		archive_local_ = "";
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

	return;
}

void job_evaluator::push_result()
{
	logger_->info() << "Trying to upload results of job...";

	// just checkout for errors
	if (job_ == nullptr) {
		logger_->warn() << "Pointer to job is null.";
		return;
	}

	// get results which will be processed from job
	auto results = job_->get_results();

	// create result directory for temporary files
	results_path_ = fs::temp_directory_path() / "isoeval" / "tmp" / "results" /
			config_->get_worker_id() / job_id_;
	fs::create_directories(results_path_);
	// define path to result yaml file and archived result
	fs::path result_path = results_path_ / "result.yml";
	fs::path archive_path = results_path_ / "result.zip";

	logger_->info() << "Building yaml results file...";
	// build yaml tree
	YAML::Node res;
	res["job-id"] = job_id_;
	for (auto &i : results) {
		if (i.second != nullptr) {
			YAML::Node node;
			node["task-id"] = i.first;
			node["exitcode"] = i.second->exitcode;
			node["time"] = i.second->time;
			node["wall-time"] = i.second->wall_time;
			node["memory"] = i.second->memory;
			node["max-rss"] = i.second->max_rss;
			//node["status"] = i.second->status; // TODO: omitted for now
			node["exitsig"] = i.second->exitsig;
			node["killed"] = i.second->killed;
			node["message"] = i.second->message;

			res["results"].push_back(node);
		}
	}

	// open output stream and write constructed yaml
	std::ofstream out(result_path.string());
	out << res;
	logger_->info() << "Yaml result file written succesfully.";

	// compress given result.yml file
	logger_->info() << "Compression of results file...";
	try {
		archivator::compress(results_path_.string(), archive_path.string());
	} catch (archive_exception &e) {
		logger_->warn() << "Results file not archived properly.";
		return;
	}
	logger_->info() << "Compression done.";

	// send archived result to file server
	submission_fileman_->set_params(result_url_, "", ""); // TODO resolve file manager problems
	submission_fileman_->put_file(archive_path.string());

	logger_->info() << "Job results uploaded succesfully.";
	return;
}

eval_response job_evaluator::evaluate (eval_request request)
{
	logger_->info() << "Request for job evaluation arrived to worker.";
	logger_->info() << "Job ID of incoming job is: " + request.job_id;

	// set all needed variables for evaluation
	job_id_ = request.job_id;
	archive_url_ = request.job_url;
	result_url_ = request.result_url;
	std::cerr << "Job ID: " << job_id_ << std::endl;
	std::cerr << "Archive url: " << archive_url_ << std::endl;
	std::cerr << "Result url: " << result_url_ << std::endl;

	try {
		download_submission();
		prepare_submission();
		build_job();
		run_job();
		push_result();
	} catch (std::exception &e) {
		logger_->warn() << "Job evaluator encountered error: " << e.what();
	}
	cleanup_evaluator();

	return eval_response(request.job_id, "OK");
	logger_->info() << "Job (" + request.job_id + ") ended.";
}

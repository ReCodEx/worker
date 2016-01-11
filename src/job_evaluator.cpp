#include "job_evaluator.h"

job_evaluator::job_evaluator(std::shared_ptr<spdlog::logger> logger,
							 std::shared_ptr<worker_config> config,
							 std::shared_ptr<file_manager_base> fileman)
	: archive_url_(), archive_local_(), job_(nullptr),
	  fileman_(fileman), logger_(logger), config_(config)
{}

job_evaluator::~job_evaluator()
{}

void job_evaluator::download_submission()
{
	// initialize all paths
	fs::path archive_url = archive_url_;
	archive_local_ = fs::temp_directory_path() / "isoeval" / "tmp" / "download" /
			config_->get_worker_id() / job_id_;
	fs::create_directories(archive_local_);

	// download a file
	fileman_->get_file(archive_url.string(), archive_local_.string());
	archive_local_ /= archive_url.filename();
	return;
}

void job_evaluator::prepare_submission()
{
	// initialize all paths
	submission_path_ = fs::temp_directory_path() / "isoeval" / "tmp" / "submission" /
			config_->get_worker_id() / job_id_;
	source_path_ = fs::temp_directory_path() / "isoeval" / config_->get_worker_id() / job_id_;
	fs::create_directories(submission_path_);
	fs::create_directories(source_path_);

	// decompress downloaded archive
	archivator::decompress(archive_local_.string(), submission_path_.string());

	// copy source codes to source code folder
	if (fs::is_directory(submission_path_)) {
		fs::recursive_directory_iterator endit;
		for (fs::recursive_directory_iterator it(submission_path_); it != endit; ++it) {
			if (fs::is_regular_file(*it)) {
				fs::copy(*it, source_path_ / it->path().filename());
			}
		}
	}

	return;
}

void job_evaluator::build_job()
{
	using namespace boost::filesystem;
	path config_path(submission_path_);
	config_path /= "config.yml";
	if (!exists(config_path)) {
		throw job_exception("Job configuration not found");
	}

	YAML::Node conf;
	try {
		conf = YAML::LoadFile(config_path.string());
	} catch (YAML::Exception e) {
		throw job_exception("Job configuration not loaded correctly: " + std::string(e.what()));
	}

	job_ = std::make_shared<job>(conf, source_path_, logger_, config_, fileman_);

	return;
}

void job_evaluator::run_job()
{
	try {
		job_->run();
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
	if (fs::exists(source_path_)) {
		fs::remove_all(source_path_);
	}

	// delete submission decompressed directory
	if (fs::exists(submission_path_)) {
		fs::remove_all(submission_path_);
	}

	// and finally delete downloaded archive
	if (fs::exists(archive_local_)) {
		fs::remove_all(archive_local_);
	}

	return;
}

void job_evaluator::push_result()
{
	// just checkout for errors
	if (job_ == nullptr) {
		return;
	}

	// get results which will be processed from job
	auto results = job_->get_results();

	// create result directory for temporary files
	fs::path results_path = fs::temp_directory_path() / "isoeval" / "tmp" / "results" /
			config_->get_worker_id() / job_id_;
	fs::create_directories(results_path);
	// define path to result yaml file and archived result
	fs::path result_path = results_path / "result.yml";
	fs::path archive_path = results_path / "result.zip";

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

	// compress given result.yml file
	try {
		archivator::compress(results_path.string(), archive_path.string());
	} catch (archive_exception &ex) {
		logger_->warn() << "Result of job (id: " + job_id_ +
						   ") was not send properly to file server: " +
						   std::string(ex.what());
		return;
	}

	// send archived result to file server
	fileman_->put_file(archive_path.string());

	return;
}

eval_response job_evaluator::evaluate (eval_request request)
{
	std::cout << request.job_url << std::endl;

	// set all needed variables for evaluation
	job_id_ = request.job_id;
	archive_url_ = request.job_url;
	result_url_ = request.result_url;

	try {
		download_submission();
		prepare_submission();
		build_job();
		run_job();
		cleanup_submission();
		push_result();
	} catch (std::exception &e) {
		logger_->warn() << "Job evaluator: " << e.what();
	}

	return eval_response(request.job_id, "OK");
}

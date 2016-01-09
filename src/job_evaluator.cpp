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
	fileman_->get_file(archive_url.filename().string(), archive_local_.string());
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
		fs::directory_iterator endit;
		for (fs::directory_iterator it(submission_path_); it != endit; ++it) {
			if (fs::is_regular_file(it->status())) {
				fs::copy(it->path(), source_path_);
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
	} catch (std::exception &ex) {
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
	// nothing to do here right now
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
	} catch (...) {}
	cleanup_submission();
	push_result();

	return eval_response(request.job_id, "OK");
}

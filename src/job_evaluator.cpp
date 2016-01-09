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
	return;
}

void job_evaluator::prepare_submission()
{
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
	} catch (std::exception ex) {
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
	return;
}

eval_response job_evaluator::evaluate (eval_request request)
{
	std::cout << request.job_url << std::endl;

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

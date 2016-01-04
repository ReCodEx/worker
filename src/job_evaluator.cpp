#include "job_evaluator.h"

job_evaluator::job_evaluator(std::shared_ptr<spdlog::logger> logger,
							 std::shared_ptr<worker_config> config,
							 std::shared_ptr<file_manager_base> fileman)
	: job_(nullptr), logger_(logger), archive_local_(), archive_url_(),
	  config_(config), fileman_(fileman)
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
	job_ = std::make_shared<job>(submission_path_, logger_, config_);
	return;
}

void job_evaluator::run_job()
{
	job_->run();
	return;
}

void job_evaluator::cleanup_submission()
{
	return;
}

void job_evaluator::push_result()
{
	return;
}

void job_evaluator::evaluate (eval_request request)
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
}

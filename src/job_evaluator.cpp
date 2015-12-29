#include "job_evaluator.h"

job_evaluator::job_evaluator(std::shared_ptr<spdlog::logger> logger,
							 std::shared_ptr<worker_config> config)
	: job_(nullptr), logger_(logger), archive_local_(), archive_url_(),
	  config_(config)
{}

void job_evaluator::run()
{
	return;
}

void job_evaluator::wait_for_submission()
{
	return;
}

void job_evaluator::eval_submission()
{
	return;
}

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
	return;
}

void job_evaluator::run_job()
{
	return;
}

void job_evaluator::cleanup_job()
{
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

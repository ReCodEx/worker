#include "job.h"

job::job(std::string submission_path,
		 std::shared_ptr<spdlog::logger> logger,
		 std::shared_ptr<worker_config> config)
	: submission_path_(submission_path),
	  logger_(logger), default_config_(config)
{
	load_config();
	build_job();
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
	return;
}

void job::build_job()
{
	return;
}

void job::cleanup_job()
{
	return;
}

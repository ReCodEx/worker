#ifndef CODEX_WORKER_JOB_EVALUATOR_HPP
#define CODEX_WORKER_JOB_EVALUATOR_HPP

#include <memory>
#include "spdlog/spdlog.h"
#include "tasks/job.h"
#include "config/worker_config.h"


/**
 *
 */
class job_evaluator {
public:
	job_evaluator() = delete;
	job_evaluator(const job_evaluator &source) = delete;
	job_evaluator& operator=(const job_evaluator &source) = delete;
	job_evaluator(const job_evaluator &&source) = delete;
	job_evaluator& operator=(const job_evaluator &&source) = delete;

	job_evaluator(std::shared_ptr<spdlog::logger> logger,
				  std::shared_ptr<worker_config> config);
	~job_evaluator();

	void run();
private:

	void wait_for_submission();
	void eval_submission();
	void download_submission();
	void prepare_submission();
	void build_job();
	void run_job();
	void cleanup_job();
	void cleanup_submission();
	void push_result();


	// PRIVATE DATA MEMBERS
	std::string archive_url_;
	std::string archive_local_;
	size_t job_id_;
	std::shared_ptr<job> job_;
	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<worker_config> config_;
};

#endif //CODEX_WORKER_JOB_EVALUATOR_HPP

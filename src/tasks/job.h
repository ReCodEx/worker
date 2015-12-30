#ifndef CODEX_WORKER_JOB_HPP
#define CODEX_WORKER_JOB_HPP

#include <vector>
#include "task_base.h"
#include "spdlog/spdlog.h"
#include "../config/worker_config.h"


/**
 *
 */
class job {
public:
	job() = delete;

	job(std::string submission_path,
		std::shared_ptr<spdlog::logger> logger,
		std::shared_ptr<worker_config> config);
	~job();

	void run();
private:

	void load_config();
	void build_job();
	void cleanup_job();


	// PRIVATE DATA MEMBERS
	YAML::Node config_;

	std::string submission_path_;

	std::vector<task_base> tasks_;
	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<worker_config> default_config_;
};

#endif //CODEX_WORKER_JOB_HPP

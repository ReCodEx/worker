#ifndef CODEX_WORKER_JOB_HPP
#define CODEX_WORKER_JOB_HPP

#include <vector>
#include <boost/filesystem.hpp>
#include "spdlog/spdlog.h"
#include "../config/worker_config.h"
#include "task_base.h"
#include "fake_task.h"
#include "../fileman/file_manager_base.h"


/**
 *
 */
class job {
public:
	job() = delete;

	job(std::string submission_path,
		std::shared_ptr<spdlog::logger> logger,
		std::shared_ptr<worker_config> config,
		std::shared_ptr<file_manager_base> fileman);
	~job();

	void run();
private:

	void load_config();
	void build_job();
	/**
	 * Cleanup after job evaluation, should be enough to delete all created files
	 */
	void cleanup_job();
	/**
	 * Prepare downloaded source codes to working directory
	 * Prepare file manager for hostname from config file
	 */
	void prepare_job();


	// PRIVATE DATA MEMBERS
	YAML::Node config_;

	std::string submission_path_;

	std::shared_ptr<file_manager_base> fileman_;

	//* Information about submission  *//
	size_t job_id_;
	std::string language_;
	std::string fileman_hostname_;
	std::string fileman_port_;
	std::string fileman_username_;
	std::string fileman_passwd_;

	/** Logical start of every job evaluation */
	std::shared_ptr<task_base> root_task_;
	/** Tasks in linear ordering prepared to evaluation */
	std::vector<std::shared_ptr<task_base>> task_queue_;
	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<worker_config> default_config_;
};

class job_exception : public std::exception {
public:
	job_exception() : what_("Generic job exception") {}
	job_exception(const std::string &what) : what_(what) {}
	virtual ~job_exception() {}
	virtual const char* what() const noexcept
	{
		return what_.c_str();
	}
protected:
	std::string what_;
};

#endif //CODEX_WORKER_JOB_HPP

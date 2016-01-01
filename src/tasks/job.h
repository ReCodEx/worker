#ifndef CODEX_WORKER_JOB_HPP
#define CODEX_WORKER_JOB_HPP

#include <vector>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "spdlog/spdlog.h"
#include "../config/worker_config.h"
#include "task_base.h"
#include "fake_task.h"
#include "external_task.h"
#include "../fileman/file_manager_base.h"
#include "../sandbox/sandbox_base.h"


/**
 *
 */
class job {
public:
	job() = delete;

	job(const YAML::Node &job_config,
		fs::path source_path,
		std::shared_ptr<spdlog::logger> logger,
		std::shared_ptr<worker_config> default_config,
		std::shared_ptr<file_manager_base> fileman);
	~job();

	void run();
private:

	void build_job(const YAML::Node &conf);
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
	fs::path source_path_;

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

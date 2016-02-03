#ifndef CODEX_WORKER_JOB_HPP
#define CODEX_WORKER_JOB_HPP

#include <vector>
#include <queue>
#include <utility>

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
#include "internal/cp_task.h"
#include "internal/mkdir_task.h"
#include "internal/rename_task.h"
#include "internal/rm_task.h"
#include "internal/archivate_task.h"
#include "internal/extract_task.h"
#include "internal/fetch_task.h"
#include "../helpers/topological_sort.h"


/**
 * Job is unit which is received from broker and should be evaluated.
 * Job is built from configuration in which all information should be provided.
 * Job building results in task tree and task queue in which task should be evaluated.
 */
class job {
public:
	job() = delete;

	/**
	 * Only way to construct a job. All variables are needed to proper function.
	 * @param job_config configuration of newly created job
	 * @param source_path path to source codes of submission
	 * @param result_path path to directory containing all results
	 * @param default_config default configuration of worker where defaults are loaded
	 * @param fileman file manager which is provided to tasks
	 * @throws job_exception if there is problem during loading of configuration
	 */
	job(const YAML::Node &job_config, fs::path source_path, fs::path result_path,
		std::shared_ptr<worker_config> default_config, std::shared_ptr<file_manager_base> fileman);
	~job();

	/**
	 * Runs all task which are sorted in task queue and get results from all of them.
	 * Should not throw an exception.
	 * @return Vector with pairs task id - task_results. Values are not @a nullptr.
	 */
	std::vector<std::pair<std::string, std::shared_ptr<task_results>>> run();

private:
	/**
	 * Init system logger for job. Resulting log will be send with other results to frontend.
	 */
	void init_logger();
	/**
	 * From given @a conf construct an evaluation tree,
	 * which includes parsing of configuration and constructing all tasks
	 * @param conf input configuration which will be loaded
	 * @throws job_exception if configuration is in bad form
	 */
	void build_job(const YAML::Node &conf);
	/**
	 * Cleanup after job evaluation, should be enough to delete all created files
	 */
	void cleanup_job();
	/**
	 * Prepare file manager for hostname from config file
	 */
	void prepare_job();


	// PRIVATE DATA MEMBERS
	fs::path source_path_;
	fs::path result_path_;

	std::shared_ptr<file_manager_base> fileman_;

	//* Information about submission  *//
	std::string job_id_;
	std::string language_;
	std::string fileman_hostname_;
	std::string fileman_port_;
	std::string fileman_username_;
	std::string fileman_passwd_;

	/** Logical start of every job evaluation */
	std::shared_ptr<task_base> root_task_;
	/** Tasks in linear ordering prepared for evaluation */
	std::vector<std::shared_ptr<task_base>> task_queue_;
	std::shared_ptr<worker_config> default_config_;

	/** Job logger */
	std::shared_ptr<spdlog::logger> logger_;
};


/**
 * Job exception class.
 */
class job_exception : public std::exception {
public:
	/**
	 * Generic job exception with no specification.
	 */
	job_exception() : what_("Generic job exception") {}
	/**
	 * Exception with some brief description.
	 * @param what textual description of a problem
	 */
	job_exception(const std::string &what) : what_(what) {}
	/**
	 * Virtual destructor.
	 */
	virtual ~job_exception() {}
	/**
	 * Return description of this exception.
	 * @return C string
	 */
	virtual const char* what() const noexcept
	{
		return what_.c_str();
	}
protected:
	std::string what_;
};

#endif //CODEX_WORKER_JOB_HPP

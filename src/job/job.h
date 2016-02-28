#ifndef CODEX_WORKER_JOB_HPP
#define CODEX_WORKER_JOB_HPP

#include <vector>
#include <queue>
#include <utility>
#include <memory>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "../config/worker_config.h"
#include "../tasks/task_base.h"
#include "../tasks/fake_task.h"
#include "../tasks/external_task.h"
#include "../fileman/file_manager_base.h"
#include "../sandbox/sandbox_base.h"
#include "../tasks/internal/cp_task.h"
#include "../tasks/internal/mkdir_task.h"
#include "../tasks/internal/rename_task.h"
#include "../tasks/internal/rm_task.h"
#include "../tasks/internal/archivate_task.h"
#include "../tasks/internal/extract_task.h"
#include "../tasks/internal/fetch_task.h"
#include "../helpers/topological_sort.h"
#include "../config/job_metadata.h"
#include "../config/task_metadata.h"


/**
 * Job is unit which is received from broker and should be evaluated.
 * Job is built from configuration in which all information should be provided.
 * Job building results in task tree and task queue in which task should be evaluated.
 */
class job
{
public:
	job() = delete;

	/**
	 * Only way to construct a job. All variables are needed to proper function.
	 * @param job_meta
	 * @param worker_conf
	 * @param working_directory Directory for temporary saving of files by tasks. Example
	 *							use case is storing isolate's meta log file.
	 * @param source_path path to source codes of submission
	 * @param result_path path to directory containing all results
	 * @param fileman file manager which is provided to tasks
	 * @throws job_exception if there is problem during loading of configuration
	 */
	job(std::shared_ptr<job_metadata> job_meta,
		std::shared_ptr<worker_config> worker_conf,
		fs::path working_directory,
		fs::path source_path,
		fs::path result_path,
		std::shared_ptr<file_manager_base> fileman);

	/**
	 * Job cleanup (if needed) is executed.
	 */
	~job();

	/**
	 * Runs all task which are sorted in task queue and get results from all of them.
	 * Should not throw an exception.
	 * @return Vector with pairs task id - task_results. Values are not @a nullptr.
	 */
	std::vector<std::pair<std::string, std::shared_ptr<task_results>>> run();

	const std::vector<std::shared_ptr<task_base>> &get_task_queue() const;

private:
	/**
	 * Check directories given during construction for existence.
	 */
	void check_job_dirs();
	/**
	 * Init system logger for job. Resulting log will be send with other results to frontend.
	 */
	void init_logger();
	/**
	 * Cleanup after job evaluation, should be enough to delete all created files
	 */
	void cleanup_job();
	/**
	 * Build job from @a job_meta_. Should be called in constructor.
	 */
	void build_job();

	/**
	 * Prepare variables which can be used in job configuration.
	 */
	void prepare_job_vars();
	/**
	 * In input string tries to find internal job config variable.
	 * If so, then replace variable with actual value and return it.
	 * @param src scanned string for variables
	 * @return new string with all variables replaced with values
	 */
	std::string parse_job_var(const std::string &src);

	// PRIVATE DATA MEMBERS
	/** Information about this job given on construction. */
	std::shared_ptr<job_metadata> job_meta_;
	/** Pointer on default worker config. */
	std::shared_ptr<worker_config> worker_config_;
	/** Directory, where tasks can create their own subfolders and temporary files. */
	fs::path working_directory_;
	/** Directory where source codes needed in job execution are stored. */
	fs::path source_path_;
	/** Directory where results and log of job are stored. */
	fs::path result_path_;
	/** File manager which will be used for fetch tasks. */
	std::shared_ptr<file_manager_base> fileman_;

	/** Variables which can be used in job configuration */
	std::map<std::string, std::string> job_variables_;

	/** Logical start of every job evaluation */
	std::shared_ptr<task_base> root_task_;
	/** Tasks in linear ordering prepared for evaluation */
	std::vector<std::shared_ptr<task_base>> task_queue_;

	/** Job logger */
	std::shared_ptr<spdlog::logger> logger_;
};


#endif // CODEX_WORKER_JOB_HPP

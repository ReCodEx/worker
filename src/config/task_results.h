#ifndef RECODEX_WORKER_TASK_RESULTS_H
#define RECODEX_WORKER_TASK_RESULTS_H

#include <string>
#include <memory>

/**
 * Return error codes of sandbox. Code names corresponds isolate's meta file error codes.
 */
enum class isolate_status { OK, RE, SG, TO, XX };

/**
 * Status of whole task after execution.
 */
enum class task_status { OK, FAILED, SKIPPED };


/**
 * Sandbox results.
 * @note Not all items must be returned from sandbox, so some defaults may aply.
 */
struct sandbox_results {
	/**
	 * Return code of sandbox.
	 * Default: 0
	 */
	int exitcode;
	/**
	 * Total run time of program inside the sandbox.
	 * Default: 0 (s)
	 */
	float time;
	/**
	 * Total run time (wall clock) of program inside the sandbox.
	 * Default: 0 (s)
	 */
	float wall_time;
	/**
	 * Amount of memory used by program inside the sandbox.
	 * Default: 0 (kB)
	 */
	size_t memory;
	/**
	 * Maximum resident set size of the process.
	 * Default: 0 (kB)
	 */
	size_t max_rss;
	/**
	 * Error code returned by sandbox.
	 * Default: OK
	 */
	isolate_status status;
	/**
	 * Signal, which killed the process.
	 * Default: 0
	 */
	int exitsig;
	/**
	 * Flag if program exited normaly or was killed.
	 * Default: false
	 */
	bool killed;
	/**
	 * Error message of the sandbox.
	 * Default: ""
	 */
	std::string message;
	/**
	 * Number of voluntary context switches.
	 * Default: 0
	 */
	size_t csw_voluntary;
	/**
	 * Number of forced context switches.
	 * Default: 0
	 */
	size_t csw_forced;

	/**
	 * Constructor with default values initialization.
	 */
	sandbox_results()
		: exitcode(0), time(0), wall_time(0), memory(0), max_rss(0), status(isolate_status::OK), exitsig(0),
		  killed(false), message(), csw_voluntary(0), csw_forced(0)
	{
	}

	/**
	 * Defaulted copy constructor.
	 */
	sandbox_results(const sandbox_results &) = default;
	/**
	 * Defaulted move constructor.
	 */
	sandbox_results(sandbox_results &&) = default;
	/**
	 * Defaulted copy assignment.
	 */
	sandbox_results &operator=(const sandbox_results &) = default;
	/**
	 * Defaulted move assignment.
	 */
	sandbox_results &operator=(sandbox_results &&) = default;
};


/**
 * Generic task result. Common for all tasks (internal and external).
 */
struct task_results {
	/**
	 * Status of task after execution.
	 * Default: OK
	 */
	task_status status;
	/**
	 * Error message if the task failed.
	 * Default: ""
	 */
	std::string error_message;
	/**
	 * Output as taken from stdout.
	 */
	std::string output_stdout;
	/**
	 * Output as taken from stderr.
	 */
	std::string output_stderr;
	/**
	 * Pointer to @ref sandbox_results for external task results.
	 * Default: nullptr (other types of tasks)
	 */
	std::unique_ptr<sandbox_results> sandbox_status;

	/**
	 * Constructor with default values initiazation.
	 */
	task_results() : status(task_status::OK), error_message(), output_stdout(), output_stderr(), sandbox_status(nullptr)
	{
	}

	/**
	 * Defaulted copy constructor.
	 */
	task_results(const task_results &) = default;
	/**
	 * Defaulted move constructor.
	 */
	task_results(task_results &&) = default;
	/**
	 * Defaulted copy assignment.
	 */
	task_results &operator=(const task_results &) = default;
	/**
	 * Defaulted move assignment.
	 */
	task_results &operator=(task_results &&) = default;
};


#endif // RECODEX_WORKER_TASK_RESULTS_H

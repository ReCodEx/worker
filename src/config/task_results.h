#ifndef CODEX_WORKER_TASK_RESULTS_H
#define CODEX_WORKER_TASK_RESULTS_H


/**
 * Return error codes of sandbox.
 */
enum class isolate_status { RE, SG, TO, XX, NOTSET };

/**
 * Sandbox results.
 * @note Not all items must be returned from sandbox, so some defaults may aply.
 */
struct task_results {
	/**
	 * Return code of sandbox.
	 * Default: 0
	 */
	int exitcode = 0;
	/**
	 * Total run time of program inside the sandbox.
	 * Default: 0 (s)
	 */
	float time = 0.0;
	/**
	 * Total run time (wall clock) of program inside the sandbox.
	 * Default: 0 (s)
	 */
	float wall_time = 0.0;
	/**
	 * Amount of memory used by program inside the sandbox.
	 * Default: 0 (kB)
	 */
	size_t memory = 0;
	/**
	 * Maximum resident set size of the process.
	 * Default: 0 (kB)
	 */
	size_t max_rss = 0;
	/**
	 * Error code returned by sandbox.
	 * Default: NOTSET
	 */
	isolate_status status = isolate_status::NOTSET;
	/**
	 * Signal, which killed the process.
	 * Default: 0
	 */
	int exitsig = 0;
	/**
	 * Flag if program exited normaly or was killed.
	 * Default: false
	 */
	bool killed = false;
	/**
	 * Error message of the sandbox.
	 * Default: ""
	 */
	std::string message;
};

#endif // CODEX_WORKER_TASK_RESULTS_H

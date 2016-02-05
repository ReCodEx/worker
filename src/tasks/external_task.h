#ifndef CODEX_WORKER_EXTERNAL_TASK_HPP
#define CODEX_WORKER_EXTERNAL_TASK_HPP

#include "task_base.h"
#include "../sandbox/isolate_sandbox.h"
#include "../sandbox/fake_sandbox.h"
#include "../config/task_results.h"


/**
 * Class which handles external tasks, aka tasks which will be executed in sandbox.
 * This class have to deal with construction of apropriate sandbox and running program in it.
 */
class external_task : public task_base {
public:
	external_task() = delete;

	/**
	 * Only way to construct external task is through this constructor.
	 * Choosing propriate sandbox and constructing it is also done here.
	 * @param worker_id unique worker identification on this machine
	 * @param id unique integer which means order in config file
	 * @param task_id unique identifier of task
	 * @param priority priority of task
	 * @param fatal if true than job will be killed immediatelly in case of error
	 * @param dependencies dependencies of this task
	 * @param binary program which will be launched
	 * @param arguments arguments for binary
	 * @param sandbox_id name of sandbox which will be used
	 * @param limits limits for sandbox
	 * @throws task_exception if @a sandbox_id is unknown
	 */
	external_task(size_t worker_id, size_t id, const std::string &task_id,
				  size_t priority, bool fatal,
				  const std::vector<std::string> &dependencies,
				  const std::string &binary, const std::vector<std::string> &arguments,
				  const std::string &sandbox_id, sandbox_limits limits);
	/**
	 * Empty right now.
	 */
	virtual ~external_task();

	/**
	 * Runs given program and parameters in constructed sandbox.
	 * @throws sandbox_exception if error occured in sandbox
	 */
	virtual void run();

	/**
	 * Get results from this particular task.
	 * It overwrite virtual function from task_base.
	 * @return
	 */
	virtual std::shared_ptr<task_results> get_result();

	/**
	 * Get sandbox_limits structure, given during construction.
	 * @return
	 */
	sandbox_limits get_limits();
private:

	/**
	 * Check if sandbox_id have counterpart in sandbox classes.
	 */
	void sandbox_check();
	/**
	 * Construct apropriate sandbox according his name give during construction.
	 */
	void sandbox_init();
	/**
	 * Destruction of internal sandbox.
	 */
	void sandbox_fini();

	/** Id of this instance of worker loaded from default configuration */
	size_t worker_id_;
	/** Name of program which will be run in sandbox */
	std::string cmd_;
	/** Cmd line arguments to program */
	std::vector<std::string> args_;
	/** Name of sandbox */
	std::string sandbox_id_;
	/** Constructed sandbox itself */
	std::shared_ptr<sandbox_base> sandbox_;
	/** Limits for sandbox in which program will be started */
	sandbox_limits limits_;
	/** Results from executed program */
	std::shared_ptr<task_results> results_;
};

#endif //CODEX_WORKER_EXTERNAL_TASK_HPP

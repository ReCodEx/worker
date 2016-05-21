#ifndef CODEX_WORKER_EXTERNAL_TASK_HPP
#define CODEX_WORKER_EXTERNAL_TASK_HPP

#include "../config/sandbox_limits.h"
#include "../sandbox/sandbox_base.h"
#include "create_params.h"
#include "spdlog/spdlog.h"
#include "task_base.h"
#include <memory>


/**
 * Class which handles external tasks, aka tasks which will be executed in sandbox.
 * This class have to deal with construction of apropriate sandbox and running program in it.
 */
class external_task : public task_base
{
public:
	external_task() = delete;

	/**
	 * Only way to construct external task is through this constructor.
	 * Choosing propriate sandbox and constructing it is also done here.
	 * @param data data to create external task class
	 * @throws task_exception if @a sandbox_id is unknown
	 */
	external_task(const create_params &data);
	/**
	 * Empty right now.
	 */
	virtual ~external_task();

	/**
	 * Runs given program and parameters in constructed sandbox.
	 * @return @ref task_results with @a sandbox_status item properly set
	 * @throws @ref sandbox_exception if error occured in sandbox
	 */
	virtual std::shared_ptr<task_results> run();

	/**
	 * Get sandbox_limits structure, given during construction.
	 * @return
	 */
	std::shared_ptr<sandbox_limits> get_limits();

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
	/** Constructed sandbox itself */
	std::shared_ptr<sandbox_base> sandbox_;
	/** Limits for sandbox in which program will be started */
	std::shared_ptr<sandbox_limits> limits_;
	/** Job system logger */
	std::shared_ptr<spdlog::logger> logger_;
	/** Directory for temporary files */
	std::string temp_dir_;
};

#endif // CODEX_WORKER_EXTERNAL_TASK_HPP

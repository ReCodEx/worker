#ifndef RECODEX_WORKER_EXTERNAL_TASK_HPP
#define RECODEX_WORKER_EXTERNAL_TASK_HPP

#include "spdlog/spdlog.h"
#include <memory>
#include "task_base.h"
#include "create_params.h"
#include "../sandbox/sandbox_base.h"
#include "../config/sandbox_limits.h"


/**
 * Class which handles external tasks, aka tasks which will be executed in sandbox.
 * This class have to deal with construction of apropriate sandbox and running program in it.
 */
class external_task : public task_base
{
public:
	/**
	 * Disabled default constructor.
	 */
	external_task() = delete;

	/**
	 * Only way to construct external task is through this constructor.
	 * Choosing propriate sandbox and constructing it, is also done here.
	 * @param data Data to create external task class.
	 * @throws task_exception if name of the sandbox in data argument is unknown.
	 */
	external_task(const create_params &data);
	/**
	 * Destructor, empty right now.
	 */
	virtual ~external_task();

	/**
	 * Runs given program and parameters in constructed sandbox.
	 * @return @ref task_results with @a sandbox_status item properly set
	 * @throws sandbox_exception if fatal error occured in sandbox
	 */
	virtual std::shared_ptr<task_results> run();

	/**
	 * Get sandbox_limits structure, given during construction.
	 * @return Restrictive limits for sandboxed program.
	 */
	std::shared_ptr<sandbox_limits> get_limits();

private:
	/**
	 * Check if sandbox name have counterpart in sandbox classes (ie. ReCodEx knows
	 * stated sandbox).
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

	/**
	 * For the given file find appropriate path outside sandbox in the directories specified in the limits.
	 * @param file file pointing inside sandbox
	 * @return path of the directory and the file outside sandbox
	 */
	fs::path find_path_outside_sandbox(std::string file);

	/**
	 * Initialize output if requested.
	 */
	void results_output_init();
	/**
	 * Get configuration limited content of the stdout and stderr and return it.
	 * @return text which was produced by sandboxed program on stdout and stderr
	 */
	std::string get_results_output();

	/** Worker default configuration */
	std::shared_ptr<worker_config> worker_config_;
	/** Constructed sandbox itself */
	std::shared_ptr<sandbox_base> sandbox_;
	/** General sandbox config */
	std::shared_ptr<sandbox_config> sandbox_config_;
	/** Limits for sandbox in which program will be started */
	std::shared_ptr<sandbox_limits> limits_;
	/** Job system logger */
	std::shared_ptr<spdlog::logger> logger_;
	/** Directory for temporary files */
	std::string temp_dir_;
	/** Directory where source codes for job are located */
	fs::path source_dir_;
	/** Directory binded to the sandbox as default working dir */
	fs::path working_dir_;
	/** After execution delete stdout file produced by sandbox */
	bool remove_stdout_ = false;
	/** After execution delete stderr file produced by sandbox */
	bool remove_stderr_ = false;
};

#endif // RECODEX_WORKER_EXTERNAL_TASK_HPP

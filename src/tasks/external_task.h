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
	~external_task() override = default;

	/**
	 * Runs given program and parameters in constructed sandbox.
	 * @return @ref task_results with @a sandbox_status item properly set
	 * @throws sandbox_exception if fatal error occured in sandbox
	 */
	std::shared_ptr<task_results> run() override;

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
	fs::path find_path_outside_sandbox(const std::string &file);

	/**
	 * If binary file provided as argument does not have executable flag, try to set it.
	 * @param binary
	 */
	void make_binary_executable(const std::string &binary);

	/**
	 * Initialize output if requested.
	 */
	void results_output_init();
	/**
	 * Get configuration limited content of the stdout and stderr and return it.
	 * @param result to which stdout and err will be assigned
	 */
	void get_results_output(std::shared_ptr<task_results> result);
	void process_results_output(
		const std::shared_ptr<task_results> &result, const fs::path &stdout_path, const fs::path &stderr_path);
	void process_carboncopy_output(const fs::path &stdout_path, const fs::path &stderr_path);

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
	/** Directory outside sandbox where task will be executed */
	fs::path evaluation_dir_;
	/** Directory binded to the sandbox as default working dir */
	fs::path sandbox_working_dir_;
	/** After execution delete stdout file produced by sandbox */
	bool remove_stdout_ = false;
	/** After execution delete stderr file produced by sandbox */
	bool remove_stderr_ = false;
};

#endif // RECODEX_WORKER_EXTERNAL_TASK_HPP

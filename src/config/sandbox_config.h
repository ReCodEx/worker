#ifndef RECODEX_WORKER_SANDBOX_CONFIG_H
#define RECODEX_WORKER_SANDBOX_CONFIG_H

#include <map>
#include "sandbox_limits.h"


/**
 * Configuration of sandbox loaded from configuration file.
 */
class sandbox_config
{
public:
	/**
	 * Name of sandbox which will be used.
	 */
	std::string name = "";
	/**
	 * Redirect standard input from given file.
	 * @note Path must be accessible from inside of sandbox.
	 */
	std::string std_input = "";
	/**
	 * Redirect standard output to given file.
	 * @note Path must be accessible from inside of sandbox.
	 */
	std::string std_output = "";
	/**
	 * Redirect standard error output to given file.
	 * @note Path must be accessible from inside of sandbox.
	 */
	std::string std_error = "";
	/**
	 * If true then stderr is redirected to stdout.
	 */
	bool stderr_to_stdout = false;
	/**
	 * If true then stdout and stderr will be written in the results.
	 */
	bool output = false;
	/**
	 * Change working directory to subdirectory inside the sandbox.
	 * @note Path must be accessible from inside of sandbox.
	 */
	std::string chdir = "";
	/**
	 * Working directory relative to the directory with the source files.
	 */
	std::string working_directory = "";
	/**
	 * Associative array of loaded limits with textual index identifying its hw group.
	 */
	std::map<std::string, std::shared_ptr<sandbox_limits>> loaded_limits;


	/**
	 * Constructor with defaults.
	 */
	sandbox_config()
	{
	}
};

#endif // RECODEX_WORKER_SANDBOX_CONFIG_H

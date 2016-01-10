#ifndef CODEX_WORKER_FILE_SANDBOX_BASE_H
#define CODEX_WORKER_FILE_SANDBOX_BASE_H

#include <memory>
#include <string>
#include <exception>
#include <map>
#include <vector>
#include "../config/sandbox_limits.h"
#include "../config/task_results.h"


/**
 * Base class for all sandbox implementations.
 */
class sandbox_base {
public:
	/**
	 * Destructor.
	 */
	virtual ~sandbox_base() {}
	/**
	 * Get sandboxed directory (to copy files inside, ...)
	 */
	virtual std::string get_dir() const { return sandboxed_dir_; }
	/**
	 * Run sandbox.
	 * @param binary Name of binary to run. Must be accessible from inside the sandbox.
	 * @param arguments Commandline arguments to the binary.
	 * @return Sandbox results.
	 */
	virtual task_results run(const std::string &binary, const std::vector<std::string> &arguments) = 0;
protected:
	std::string sandboxed_dir_;
};


/**
 * Common exception for all sandbox implementations.
 */
class sandbox_exception : public std::exception {
public:
	/**
	 * Default constructor.
	 */
	sandbox_exception() : what_("Generic sandbox exception") {}
	/**
	 * Constructor with custom message.
	 * @param what Custom message.
	 */
	sandbox_exception(const std::string &what) : what_(what) {}
	/**
	 * Destructor.
	 */
	virtual ~sandbox_exception() {}
	/**
	 * Get message.
	 */
	virtual const char* what() const noexcept
	{
		return what_.c_str();
	}
protected:
	std::string what_;
};

#endif // CODEX_WORKER_FILE_SANDBOX_BASE_H

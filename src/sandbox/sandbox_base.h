#ifndef RECODEX_WORKER_FILE_SANDBOX_BASE_H
#define RECODEX_WORKER_FILE_SANDBOX_BASE_H

#include <memory>
#include <string>
#include <exception>
#include <map>
#include <vector>
#include <sstream>
#include "spdlog/spdlog.h"
#include "../config/sandbox_limits.h"
#include "../config/task_results.h"
#include "../helpers/format.h"


/**
 * Base class for all sandbox implementations.
 */
class sandbox_base
{
public:
	/**
	 * Destructor.
	 */
	virtual ~sandbox_base()
	{
	}
	/**
	 * Get sandboxed directory (to copy files inside, ...)
	 */
	virtual std::string get_dir() const
	{
		return sandboxed_dir_;
	}
	/**
	 * Run sandbox.
	 * @param binary Name of binary to run. Must be accessible from inside the sandbox.
	 * @param arguments Commandline arguments to the binary.
	 * @return Sandbox results.
	 */
	virtual sandbox_results run(const std::string &binary, const std::vector<std::string> &arguments) = 0;

protected:
	/**
	 * Path to sandboxed directory.
	 * @warning Must be set in constructor of child class.
	 */
	std::string sandboxed_dir_;
};


/**
 * Common exception for all sandbox implementations.
 */
class sandbox_exception : public std::exception
{
public:
	/**
	 * Default constructor.
	 */
	sandbox_exception() : what_("Generic sandbox exception")
	{
	}
	/**
	 * Constructor with custom error message.
	 * @param what Custom message.
	 */
	sandbox_exception(const std::string &what) : what_(what)
	{
	}
	/**
	 * Destructor.
	 */
	virtual ~sandbox_exception()
	{
	}
	/**
	 * Get message describing the issue.
	 */
	virtual const char *what() const noexcept
	{
		return what_.c_str();
	}

protected:
	/** Error message. */
	std::string what_;
};


template <typename... T> void log_and_throw(std::shared_ptr<spdlog::logger> logger, T... args)
{
	std::ostringstream oss;
	helpers::format(oss, args...);
	const auto message = oss.str();
	logger->warn(message);
	throw sandbox_exception(message);
}


#endif // RECODEX_WORKER_FILE_SANDBOX_BASE_H

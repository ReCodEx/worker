#ifndef RECODEX_WORKER_FILE_SANDBOX_BASE_H
#define RECODEX_WORKER_FILE_SANDBOX_BASE_H

#include <memory>
#include <string>
#include <exception>
#include <vector>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include "spdlog/spdlog.h"
#include "config/sandbox_config.h"
#include "config/sandbox_limits.h"
#include "config/task_results.h"
#include "helpers/format.h"


/**
 * Base class for all sandbox implementations.
 *
 * Sandbox is used for security of system running untrusted program. They impose
 * sets restrictions to the application like time limit, memory limit or accessible
 * files. When any of the limits are reached, the program inside sandbox is killed.
 *
 * This is a base class for different sandbox implementations that may be used by the worker,
 * but the common code is shared in this class. At present, we only support cg-based sandboxes
 * for linux systems which is reflected in this interface.
 */
class sandbox_base
{
public:
	/**
	 * Destructor.
	 */
	virtual ~sandbox_base() = default;

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
	virtual sandbox_results execute_in_sandbox(const std::string &binary, const std::vector<std::string> &arguments);

protected:
	/**
	 * Path to sandboxed directory.
	 * @warning Must be set in constructor of child class.
	 */
	std::string sandboxed_dir_;

	/** General sandbox configuration */
	std::shared_ptr<sandbox_config> sandbox_config_;

	/** Limits for sandboxed program */
	sandbox_limits limits_;

	/** Logger */
	std::shared_ptr<spdlog::logger> logger_;

	/** Identifier of this sandbox's instance. Must be unique on each server. */
	std::size_t id_;

	/** Path to temporary directory used by sandboxes. Subdir with "id_" value will be created. */
	std::string temp_dir_;

	/** Maximum time to run the separate sandbox process */
	int max_timeout_;

	/** Path to the directory containing sources moved to sandbox and back */
	std::string data_dir_;

	/** Name of sandbox binary (also used to identify the sandbox in logs) */
	std::string sandbox_binary_;

	/**
	 * Constructor is protected, derived classes should make it public.
	 * @param sandbox_config General sandbox configuration.
	 * @param limits Limits for current command.
	 * @param id Number of current worker. This must be unique for each worker on one machine!
	 * @param temp_dir Directory to store temporary files (generated isolate's meta log)
	 * @param data_dir Directory containing sources which will be copied into sandbox
	 * @param logger Set system logger (optional).
	 * @param sandbox_binary Name of the sandbox binary (CLI executable)
	 */
	sandbox_base(std::shared_ptr<sandbox_config> sandbox_config,
		sandbox_limits limits,
		std::size_t id,
		const std::string &temp_dir,
		const std::string &data_dir,
		const std::string &sandbox_binary,
		std::shared_ptr<spdlog::logger> logger = nullptr);

	/** Initialize the sandbox */
	virtual void sandbox_init() = 0;

	/** Run isolate evaluation with sandboxed program inside. */
	virtual void sandbox_run(const std::string &binary, const std::vector<std::string> &arguments) = 0;

	/** Cleanup the sandbox after the evaluation */
	virtual void sandbox_cleanup() = 0;

	/** Extract execution results from the sandbox (measurements, errors, ...). */
	virtual sandbox_results extract_results() = 0;
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
	~sandbox_exception() override = default;

	/**
	 * Get message describing the issue.
	 */
	const char *what() const noexcept override
	{
		return what_.c_str();
	}

protected:
	/** Error message. */
	std::string what_;
};


/**
 * Helper function that logs a message and throws an exception with the same message.
 */
template <typename... T> void log_and_throw(std::shared_ptr<spdlog::logger> logger, T... args)
{
	std::ostringstream oss;
	helpers::format(oss, args...);
	const auto message = oss.str();
	logger->warn(message);
	throw sandbox_exception(message);
}


/**
 * Helper function that moves directory from "from" to "to" and throws an exception on failure.
 */
void move_or_throw(std::shared_ptr<spdlog::logger> logger, const std::string &from, const std::string &to);


/**
 * Helper class that wraps a pipe between a child and a parent process, allowing the child to send lines to the parent.
 */
class sandbox_log_pipe
{
	int read_fd_, write_fd_; // -1 = closed
	std::shared_ptr<spdlog::logger> logger_;

public:
	sandbox_log_pipe(std::shared_ptr<spdlog::logger> logger);
	~sandbox_log_pipe();

	/**
	 * This is called in the child process to redirect stdout or stderr to the pipe.
	 * E.g., calling `child_dup_to_fd(1);` will redirect stdout to the pipe.
	 * This should be called only once!
	 */
	void child_dup_to_fd(int fd);

	/**
	 * This is called in the parent process to get a stream to read from the pipe.
	 * This should be called only once!
	 */
	boost::iostreams::stream<boost::iostreams::file_descriptor_source> parent_read_stream();
};


#endif // RECODEX_WORKER_FILE_SANDBOX_BASE_H

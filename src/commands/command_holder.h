#ifndef CODEX_WORKER_COMMANDS_BASE_H
#define CODEX_WORKER_COMMANDS_BASE_H

#include "../config/worker_config.h"
#include "../helpers/create_logger.h"
#include "../job/job_evaluator_base.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <zmq.hpp>


/**
 * Broker connection commands specific context. For more info see @ref command_holder.
 * @warning This class must have proper copy constructor (at least automaticaly generated).
 */
template <typename proxy> class broker_connection_context
{
public:
	/** Socket for communication. */
	std::shared_ptr<proxy> sockets;
	std::shared_ptr<const worker_config> config;
};

/**
 * Job client commands specific context. For more info see @ref command_holder.
 * @warning This class must have proper copy constructor (at least automaticaly generated).
 */
class job_client_context
{
public:
	/** Pointer to instance of @ref job_evaluator class. */
	std::shared_ptr<job_evaluator_base> evaluator;
	/** Reference to ZeroMQ socket for communicating with main thread of worker. */
	zmq::socket_t &socket;
};

/**
 * Context for all commands. This class is templated (and inherited) by dependent part of context for every use-case.
 * Commands processed in "main" thread (eq. in @ref broker_connection class) has dependent part of context
 * @ref broker_connection_context, commands in "job" thread (eq. in @ref job_receiver class) has @ref job_client_context
 * as dependent part of context. For more info, see @ref command_holder class.
 */
template <typename context_t> class command_context : public context_t
{
public:
	/**
	 * Constructor.
	 * @param dependent_context Instance of dependent part of context for commands used for initialization. Note that
	 *	copy constructor must be present for proper work.
	 * @param logger System logger.
	 */
	command_context(const context_t &dependent_context, std::shared_ptr<spdlog::logger> logger)
		: context_t(dependent_context), logger(logger)
	{
		if (this->logger == nullptr) {
			logger = helpers::create_null_logger();
		}
	}
	/** System logger. */
	std::shared_ptr<spdlog::logger> logger;
};


/**
 * Command holder.
 *
 * This class can handle, register and execute commands with corresponding callbacks. Command is @a std::string,
 * callback needs to be "void callback(const std::vector<std::string> &args, const command_context<context_t> &context".
 * @a context_t is templated argument, which specifies different requirements in different usages. Every context has
 * pointer to system logger, commands in "main" thread (@ref broker_connection) has additional members as in
 * @ref broker_connection_context and commands in "job" thread (@ref job_receiver) has additional members as in
 * @ref job_client_context.
 */
template <typename context_t> class command_holder
{
public:
	/** Type of callback function for easier use. */
	typedef std::function<void(const std::vector<std::string> &, const command_context<context_t> &)> callback_fn;
	/** Constructor with initialization of dependent (templated) part of context and logger. */
	command_holder(const context_t &dependent_context, std::shared_ptr<spdlog::logger> logger = nullptr)
		: context_(dependent_context, logger)
	{
	}
	/**
	 * Invoke registered callback for given command (if any).
	 * @param command String reprezentation of command for processing.
	 * @param args Arguments for callback function.
	 */
	void call_function(const std::string &command, const std::vector<std::string> &args)
	{
		auto it = functions_.find(command);
		if (it != functions_.end()) {
			(it->second)(args, context_);
		}
	}
	/**
	 * Register new command with a callback.
	 * @param command String reprezentation of command.
	 * @param callback Function to call when this command occurs.
	 * @return @a true if add successful, @a false othewise
	 */
	bool register_command(const std::string &command, callback_fn callback)
	{
		auto ret = functions_.emplace(command, callback);
		return ret.second;
	}

protected:
	/** Container for <command, callback> pairs with fast searching. */
	std::map<std::string, callback_fn> functions_;

private:
	/** Inner context to be passed to callback when invoked. */
	const command_context<context_t> context_;
};

#endif // CODEX_WORKER_COMMANDS_BASE_H

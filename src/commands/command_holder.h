#ifndef CODEX_WORKER_COMMANDS_BASE_H
#define CODEX_WORKER_COMMANDS_BASE_H

#include <memory>
#include <string>
#include <functional>
#include <map>
#include <zmq.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "../job/job_evaluator.h"

/** Broker connection commands specific context */
template <typename proxy>
class broker_connection_context {
public:
	std::shared_ptr<proxy> sockets;
};

/** Job client commands specific context */
class job_client_context {
public:
	std::shared_ptr<job_evaluator> evaluator;
	zmq::socket_t &socket;
};

/** Context for commands in broker_connection.h */
template <typename context_t> class command_context : public context_t
{
public:
	command_context(
		const context_t &dependent_context, std::shared_ptr<spdlog::logger> logger)
		: context_t(dependent_context), logger(logger)
	{
		if (this->logger == nullptr) {
			// Create logger manually to avoid global registration of logger
			auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
			this->logger = std::make_shared<spdlog::logger>("command_context_nolog", sink);
			// Set loglevel to 'off' cause no logging
			this->logger->set_level(spdlog::level::off);
		}
	}
	//std::shared_ptr<context_t> dependent_context;
	std::shared_ptr<spdlog::logger> logger;
};


/** Command holder for broker_connection.h */
template <typename context_t> class command_holder
{
public:
	typedef std::function<void(const std::vector<std::string> &, const command_context<context_t> &)> callback_fn;
	command_holder(const context_t &dependent_context,
		std::shared_ptr<spdlog::logger> logger = nullptr)
		: context_(dependent_context, logger)
	{
	}
	void call_function(const std::string &command, const std::vector<std::string> &args)
	{
		auto it = functions_.find(command);
		if (it != functions_.end()) {
			(it->second)(args, context_);
		}
	}
	bool register_command(const std::string &command, callback_fn callback)
	{
		auto ret = functions_.emplace(command, callback);
		return ret.second;
	}

protected:
	std::map<std::string, callback_fn> functions_;

private:
	const command_context<context_t> context_;
};

#endif // CODEX_WORKER_COMMANDS_BASE_H

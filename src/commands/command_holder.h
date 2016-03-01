#ifndef CODEX_WORKER_COMMANDS_BASE_H
#define CODEX_WORKER_COMMANDS_BASE_H

#include <memory>
#include <string>
#include <functional>
#include <map>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"

/** Context for commands in broker_connection.h */
template <typename proxy> class command_context
{
public:
	command_context(
		std::shared_ptr<proxy> sockets, std::shared_ptr<spdlog::logger> logger)
		: sockets(sockets), logger(logger)
	{
		if (this->logger == nullptr) {
			// Create logger manually to avoid global registration of logger
			auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
			this->logger = std::make_shared<spdlog::logger>("broker_command_context_nolog", sink);
			// Set loglevel to 'off' cause no logging
			this->logger->set_level(spdlog::level::off);
		}
	}
	std::shared_ptr<proxy> sockets;
	std::shared_ptr<spdlog::logger> logger;
};


/** Command holder for broker_connection.h */
template <typename proxy> class command_holder
{
public:
	typedef std::function<void(const std::vector<std::string> &, const command_context<proxy> &)> callback_fn;
	command_holder(std::shared_ptr<proxy> sockets,
		std::shared_ptr<spdlog::logger> logger = nullptr)
		: context_(sockets, logger)
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
	const command_context<proxy> context_;
};

#endif // CODEX_WORKER_COMMANDS_BASE_H

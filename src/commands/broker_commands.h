#ifndef RECODEX_WORKER_BROKER_COMMANDS_H
#define RECODEX_WORKER_BROKER_COMMANDS_H

#include "command_holder.h"

/**
 * Commands from broker.
 * Commands originated from broker. See @ref command_holder and @ref broker_connection.
 */
namespace broker_commands
{

	/** Eval command */
	template <typename context_t>
	void process_eval(const std::vector<std::string> &args, const command_context<context_t> &context)
	{
		context.sockets->send_jobs(args);
	}

	template <typename context_t>
	void process_intro(const std::vector<std::string> &args, const command_context<context_t> &context)
	{
		std::vector<std::string> reply = {"init", context.config->get_hwgroup()};

		for (auto &it : context.config->get_headers()) {
			reply.push_back(it.first + "=" + it.second);
		}

		context.sockets->send_broker(reply);
	}
}

#endif // RECODEX_WORKER_BROKER_COMMANDS_H

#ifndef RECODEX_WORKER_BROKER_COMMANDS_H
#define RECODEX_WORKER_BROKER_COMMANDS_H

#include "command_holder.h"

/**
 * Commands from broker.
 * Commands originated from broker. See @ref command_holder and @ref broker_connection.
 */
namespace broker_commands
{
	/**
	 * Command eval was received from broker, send it to "job" thread.
	 * @param args received multipart message with leading command
	 * @param context command context of command holder
	 */
	template <typename context_t>
	void process_eval(const std::vector<std::string> &args, const command_context<context_t> &context)
	{
		context.sockets->send_jobs(args);
	}

	/**
	 * Intro command arrived from broker, send him back init message with headers and hwgroup.
	 * @param args received multipart message with leading command
	 * @param context command context of command holder
	 */
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

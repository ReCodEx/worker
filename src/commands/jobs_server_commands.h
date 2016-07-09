#ifndef RECODEX_WORKER_JOBS_SERVER_COMMANDS_H
#define RECODEX_WORKER_JOBS_SERVER_COMMANDS_H

#include "command_holder.h"

/**
 * Commands from "job" thread to "main" thread.
 * Commands originated from worker job thread (@ref job_receiver) to main receiver thread (@ref broker_connection).
 * For more info, see @ref command_holder and @ref broker_connection.
 */
namespace jobs_server_commands
{

	/**
	 * Done command arrived from "job" thread, this information has to be sent back to broker.
	 * @param args received multipart message without leading command
	 * @param context command context of command holder
	 */
	template <typename context_t>
	void process_done(const std::vector<std::string> &args, const command_context<context_t> &context)
	{
		context.sockets->send_broker(args);
	}
}

#endif // RECODEX_WORKER_JOBS_SERVER_COMMANDS_H

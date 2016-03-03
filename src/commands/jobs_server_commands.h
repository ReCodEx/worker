#ifndef CODEX_WORKER_JOBS_SERVER_COMMANDS_H
#define CODEX_WORKER_JOBS_SERVER_COMMANDS_H

#include "command_holder.h"

/**
 * Commands from "main" thread to "job" thread.
 * Commands originated from worker main receiver thread (@ref broker_connection) to job thread (@ref job_receiver).
 * For more info, see @ref command_holder and @ref broker_connection.
 */
namespace jobs_server_commands
{

	/** Done command */
	template <typename context_t>
	void process_done(const std::vector<std::string> &args, const command_context<context_t> &context)
	{
		context.sockets->send_broker(args);
	}
}

#endif // CODEX_WORKER_JOBS_SERVER_COMMANDS_H

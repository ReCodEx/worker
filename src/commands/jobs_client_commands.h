#ifndef RECODEX_WORKER_JOBS_CLIENT_COMMANDS_H
#define RECODEX_WORKER_JOBS_CLIENT_COMMANDS_H

#include "command_holder.h"
#include "../helpers/zmq_socket.h"
#include "../eval_request.h"
#include "../eval_response.h"

/**
 * Commands from worker "main" thread.
 * Commands originated from this worker. See @ref command_holder, @ref broker_connection and @ref job_receiver.
 */
namespace jobs_client_commands
{

	/**
	 * From "main" thread arrived eval command.
	 * Received job id and other information are handed over for evaluation,
	 * after this done reply is sent back to "main" thread.
	 * @param args received multipart message with leading command
	 * @param context command context of command holder
	 */
	template <typename context_t>
	void process_eval(const std::vector<std::string> &args, const command_context<context_t> &context)
	{
		if (args.size() == 4) {
			context.logger->info("Job-receiver: Job evaluating request received.");

			eval_response response = context.evaluator->evaluate(eval_request(args[1], args[2], args[3]));
			std::vector<std::string> reply = {"done", response.job_id, response.result, response.message};

			helpers::send_through_socket(context.socket, reply);
			context.logger->info("Job-receiver: Job evaluated and respond sent.");
		} else {
			context.logger->warn("Job-receiver: Eval command with wrong number of arguments.");
		}
	}
} // namespace jobs_client_commands

#endif // RECODEX_WORKER_JOBS_CLIENT_COMMANDS_H

#ifndef CODEX_WORKER_BROKER_COMMANDS_H
#define CODEX_WORKER_BROKER_COMMANDS_H

#include "command_holder.h"

namespace broker_commands {

template <typename context_t>
void process_eval(const std::vector<std::string> &args, const command_context<context_t> &context)
{
	context.sockets->send_jobs(args);
}


}

#endif // CODEX_WORKER_BROKER_COMMANDS_H

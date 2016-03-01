#ifndef CODEX_WORKER_JOBS_SERVER_COMMANDS_H
#define CODEX_WORKER_JOBS_SERVER_COMMANDS_H

#include "command_holder.h"

namespace jobs_server_commands {

template <typename proxy>
void process_done(const std::vector<std::string> &args, const command_context<proxy> &context)
{
	context.sockets->send_broker(args);
}


}

#endif // CODEX_WORKER_JOBS_SERVER_COMMANDS_H

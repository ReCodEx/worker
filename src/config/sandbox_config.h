#ifndef CODEX_WORKER_SANDBOX_CONFIG_H
#define CODEX_WORKER_SANDBOX_CONFIG_H

#include "sandbox_limits.h"


/**
 *
 */
class sandbox_config
{
public:
	std::string name = "";
	std::string std_input = "";
	std::string std_output = "";
	std::string std_error = "";
	std::map<std::string, std::shared_ptr<sandbox_limits>> limits;
};

#endif //CODEX_WORKER_SANDBOX_CONFIG_H

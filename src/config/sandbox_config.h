#ifndef CODEX_WORKER_SANDBOX_CONFIG_H
#define CODEX_WORKER_SANDBOX_CONFIG_H

#include "sandbox_limits.h"
#include <map>
#include <memory>


/**
 * Configuration of sandbox loaded from configuration file.
 */
class sandbox_config
{
public:
	/** Name of sandbox which will be used. */
	std::string name = "";
	/** Associative array of loaded limits with textual index identifying its hw group. */
	std::map<std::string, std::shared_ptr<sandbox_limits>> loaded_limits;
};

#endif // CODEX_WORKER_SANDBOX_CONFIG_H

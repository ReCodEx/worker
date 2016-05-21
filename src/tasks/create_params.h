#ifndef CODEX_WORKER_CREATE_PARAMS_H
#define CODEX_WORKER_CREATE_PARAMS_H

#include "../config/sandbox_limits.h"
#include "../config/task_metadata.h"
#include <memory>
#include <spdlog/spdlog.h>

/** data for proper construction of @ref external_task class */
struct create_params {
	/** unique worker identification on this machine */
	size_t worker_id;
	/** unique integer which means order in config file */
	size_t id;
	/** structure containing information loaded about task */
	std::shared_ptr<task_metadata> task_meta;
	/** limits for sandbox */
	std::shared_ptr<sandbox_limits> limits;
	/** job system logger */
	std::shared_ptr<spdlog::logger> logger;
	/** directory for optional saving temporary files during execution */
	const std::string &temp_dir;
};


#endif // CODEX_WORKER_CREATE_PARAMS_H

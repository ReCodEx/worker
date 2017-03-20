#ifndef RECODEX_WORKER_CREATE_PARAMS_H
#define RECODEX_WORKER_CREATE_PARAMS_H

#include <memory>
#include <spdlog/spdlog.h>
#include "../config/worker_config.h"
#include "../config/sandbox_config.h"
#include "../config/sandbox_limits.h"
#include "../config/task_metadata.h"

/** data for proper construction of @ref external_task class */
struct create_params {
	/** unique worker identification on this machine */
	std::shared_ptr<worker_config> worker_conf;
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
	/** directory where source files given by users are located */
	fs::path source_path;
	/** working directory which points inside sandbox */
	fs::path working_path;
};


#endif // RECODEX_WORKER_CREATE_PARAMS_H

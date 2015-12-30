#ifndef CODEX_WORKER_LOG_CONFIG_H
#define CODEX_WORKER_LOG_CONFIG_H

#include "spdlog/spdlog.h"


/**
 * Structure which stores all information needed to initialize logger.
 */
struct log_config {
public:
	std::string log_path_ = "/tmp/recodex/";
	std::string log_basename_ = "worker";
	std::string log_suffix_ = "log";
	spdlog::level::level_enum log_level = spdlog::level::debug;
	int log_file_size = 1024 * 1024;
	int log_files_count = 3;
};

#endif //CODEX_WORKER_LOG_CONFIG_H

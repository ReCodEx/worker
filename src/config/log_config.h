#ifndef CODEX_WORKER_LOG_CONFIG_H
#define CODEX_WORKER_LOG_CONFIG_H

#include "spdlog/spdlog.h"


/**
 * Structure which stores all information needed to initialize logger.
 */
struct log_config {
public:
	std::string log_path = "/tmp/recodex/";
	std::string log_basename = "worker";
	std::string log_suffix = "log";
	std::string log_level = "debug";
	int log_file_size = 1024 * 1024;
	int log_files_count = 3;

	static spdlog::level::level_enum get_level(const std::string &lev)
	{
		if (lev == "emerg") {
			return spdlog::level::emerg;
		} else if (lev == "warn") {
			return spdlog::level::warn;
		}

		return spdlog::level::debug;
	}
};

#endif //CODEX_WORKER_LOG_CONFIG_H

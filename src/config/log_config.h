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
		} else if (lev == "info") {
			return spdlog::level::info;
		}

		return spdlog::level::debug;
	}

	bool operator==(const log_config &second) const
	{
		if (log_path != second.log_path ||
				log_basename != second.log_basename ||
				log_suffix != second.log_suffix ||
				log_level != second.log_level ||
				log_file_size != second.log_file_size ||
				log_files_count != second.log_files_count) {
			return false;
		}

		return true;
	}

	bool operator!=(const log_config &second) const
	{
		return !((*this) == second);
	}
};

#endif //CODEX_WORKER_LOG_CONFIG_H

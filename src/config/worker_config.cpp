#include "worker_config.h"
#include "helpers/config.h"

worker_config::worker_config() = default;

worker_config::worker_config(const YAML::Node &config)
{
	try {
		if (config["broker-uri"] && config["broker-uri"].IsScalar()) {
			broker_uri_ = config["broker-uri"].as<std::string>();
		} else {
			throw config_error("Item broker-uri not defined properly");
		}

		if (config["broker-ping-interval"] && config["broker-ping-interval"].IsScalar()) {
			broker_ping_interval_ = std::chrono::milliseconds(config["broker-ping-interval"].as<std::size_t>());
		}

		if (config["max-broker-liveness"] && config["max-broker-liveness"].IsScalar()) {
			max_broker_liveness_ = config["max-broker-liveness"].as<std::size_t>();
		}

		if (!config["headers"].IsMap()) { throw config_error("Headers are not a map"); }

		for (auto entry : config["headers"]) {
			if (!entry.first.IsScalar()) { throw config_error("A header key is not scalar"); }

			if (entry.second.IsSequence()) {
				for (auto value : entry.second) {
					if (!value.IsScalar()) { throw config_error("A header value is not scalar"); }

					headers_.insert(std::make_pair(entry.first.as<std::string>(), value.as<std::string>()));
				}
			} else if (entry.second.IsScalar()) {
				headers_.insert(std::make_pair(entry.first.as<std::string>(), entry.second.as<std::string>()));
			} else {
				throw config_error("A header value is not scalar");
			}
		}

		// load hwgroup
		if (config["hwgroup"] && config["hwgroup"].IsScalar()) {
			hwgroup_ = config["hwgroup"].as<std::string>();
		} else {
			throw config_error("Item hwgroup not defined properly");
		}

		// load file-cache item
		if (config["file-cache"] && config["file-cache"].IsMap()) {
			auto &cache = config["file-cache"];

			if (cache["cache-dir"] && cache["cache-dir"].IsScalar()) {
				cache_dir_ = config["file-cache"]["cache-dir"].as<std::string>();
			}
		}

		// load worker-id
		if (config["worker-id"] && config["worker-id"].IsScalar()) {
			worker_id_ = config["worker-id"].as<std::size_t>();
		} else {
			throw config_error("Item worker-id not defined properly");
		}

		// load worker-description
		if (config["worker-description"] && config["worker-description"].IsScalar()) {
			worker_description_ = config["worker-description"].as<std::string>();
		} // can be omitted... no throw

		// load working directory path
		if (config["working-directory"] && config["working-directory"].IsScalar()) {
			working_directory_ = config["working-directory"].as<std::string>();
		} // can be omitted... no throw

		// load file-managers
		if (config["file-managers"] && config["file-managers"].IsSequence()) {
			for (auto &fileman : config["file-managers"]) {
				fileman_config fileman_conf;
				if (fileman.IsMap()) {
					if (fileman["hostname"] && fileman["hostname"].IsScalar()) {
						fileman_conf.remote_url = fileman["hostname"].as<std::string>();
					} // no throw... can be omitted
					if (fileman["username"] && fileman["username"].IsScalar()) {
						fileman_conf.username = fileman["username"].as<std::string>();
					} // no throw... can be omitted
					if (fileman["password"] && fileman["password"].IsScalar()) {
						fileman_conf.password = fileman["password"].as<std::string>();
					} // no throw... can be omitted
				} // no throw... can be omitted

				filemans_configs_.push_back(fileman_conf);
			}
		} else {
			throw config_error("File managers not defined properly");
		}

		// load logger
		if (config["logger"] && config["logger"].IsMap()) {
			if (config["logger"]["file"] && config["logger"]["file"].IsScalar()) {
				fs::path tmp = config["logger"]["file"].as<std::string>();
				log_config_.log_basename = tmp.filename().string();
				log_config_.log_path = tmp.parent_path().string();
			} // no throw... can be omitted
			if (config["logger"]["level"] && config["logger"]["level"].IsScalar()) {
				log_config_.log_level = config["logger"]["level"].as<std::string>();
			} // no throw... can be omitted
			if (config["logger"]["max-size"] && config["logger"]["max-size"].IsScalar()) {
				log_config_.log_file_size = config["logger"]["max-size"].as<std::size_t>();
			} // no throw... can be omitted
			if (config["logger"]["rotations"] && config["logger"]["rotations"].IsScalar()) {
				log_config_.log_files_count = config["logger"]["rotations"].as<std::size_t>();
			} // no throw... can be omitted
		} // no throw... can be omitted

		// load limits
		if (config["limits"] && config["limits"].IsMap()) {
			auto limits = config["limits"];
			if (limits["time"] && limits["time"].IsScalar()) {
				limits_.cpu_time = limits["time"].as<float>();
			} // no throw... can be omitted
			if (limits["wall-time"] && limits["wall-time"].IsScalar()) {
				limits_.wall_time = limits["wall-time"].as<float>();
			} // no throw... can be omitted
			if (limits["extra-time"] && limits["extra-time"].IsScalar()) {
				limits_.extra_time = limits["extra-time"].as<float>();
			} // no throw... can be omitted
			if (limits["stack-size"] && limits["stack-size"].IsScalar()) {
				limits_.stack_size = limits["stack-size"].as<std::size_t>();
			} // no throw... can be omitted
			if (limits["memory"] && limits["memory"].IsScalar()) {
				limits_.memory_usage = limits["memory"].as<std::size_t>();
			} // no throw... can be omitted
			if (limits["extra-memory"] && limits["extra-memory"].IsScalar()) {
				limits_.extra_memory = limits["extra-memory"].as<std::size_t>();
			} // no throw... can be omitted
			if (limits["parallel"] && limits["parallel"].IsScalar()) {
				limits_.processes = limits["parallel"].as<std::size_t>();
			} // no throw... can be omitted
			if (limits["disk-quotas"] && limits["disk-quotas"].IsScalar()) {
				limits_.disk_quotas = limits["disk-quotas"].as<bool>();
			} // no throw... can be omitted
			if (limits["disk-size"] && limits["disk-size"].IsScalar()) {
				limits_.disk_size = limits["disk-size"].as<std::size_t>();
			} // no throw... can be omitted
			if (limits["disk-files"] && limits["disk-files"].IsScalar()) {
				limits_.disk_files = limits["disk-files"].as<std::size_t>();
			} // no throw... can be omitted

			try {
				auto bound_dirs = helpers::get_bind_dirs(limits);
				limits_.add_bound_dirs(bound_dirs);
			} catch (helpers::config_exception &e) {
				throw config_error(e.what());
			}

			if (limits["environ-variable"] && limits["environ-variable"].IsMap()) {
				for (const auto &var : limits["environ-variable"]) {
					limits_.environ_vars.emplace_back(var.first.as<std::string>(), var.second.as<std::string>());
				}
			} // no throw... can be omitted

		} else {
			throw config_error("Map of limits not defined properly");
		}

		// load max-output-length
		if (config["max-output-length"] && config["max-output-length"].IsScalar()) {
			max_output_length_ = config["max-output-length"].as<std::size_t>();
		} else {
			throw config_error("Item max-output-length not defined properly");
		}

		// load max-carboncopy-length
		if (config["max-carboncopy-length"] && config["max-carboncopy-length"].IsScalar()) {
			max_carboncopy_length_ = config["max-carboncopy-length"].as<std::size_t>();
		} else {
			throw config_error("Item max-carboncopy-length not defined properly");
		}

		// load cleanup-submission
		if (config["cleanup-submission"] && config["cleanup-submission"].IsScalar()) {
			cleanup_submission_ = config["cleanup-submission"].as<bool>();
		} else {
			throw config_error("Item cleanup-submission not defined properly");
		}

	} catch (YAML::Exception &ex) {
		throw config_error("Default worker configuration was not loaded: " + std::string(ex.what()));
	}
}

worker_config::~worker_config() = default;

size_t worker_config::get_worker_id() const
{
	return worker_id_;
}

const std::string &worker_config::get_worker_description() const
{
	return worker_description_;
}


const std::string &worker_config::get_working_directory() const
{
	return working_directory_;
}

const std::string &worker_config::get_broker_uri() const
{
	return broker_uri_;
}

const worker_config::header_map_t &worker_config::get_headers() const
{
	return headers_;
}

const std::string &worker_config::get_hwgroup() const
{
	return hwgroup_;
}

const log_config &worker_config::get_log_config() const
{
	return log_config_;
}

const std::vector<fileman_config> &worker_config::get_filemans_configs() const
{
	return filemans_configs_;
}

const sandbox_limits &worker_config::get_limits() const
{
	return limits_;
}

const std::string &worker_config::get_cache_dir() const
{
	return cache_dir_;
}

size_t worker_config::get_max_broker_liveness() const
{
	return max_broker_liveness_;
}

std::chrono::milliseconds worker_config::get_broker_ping_interval() const
{
	return broker_ping_interval_;
}

size_t worker_config::get_max_output_length() const
{
	return max_output_length_;
}

size_t worker_config::get_max_carboncopy_length() const
{
	return max_carboncopy_length_;
}

bool worker_config::get_cleanup_submission() const
{
	return cleanup_submission_;
}

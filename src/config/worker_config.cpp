#include "worker_config.h"

worker_config::worker_config ()
{
}

worker_config::worker_config (const YAML::Node &config)
{
	try {
		if (config["broker-uri"] && config["broker-uri"].IsScalar()) {
			broker_uri_ = config["broker-uri"].as<std::string>();
		} else { throw config_error("Item broker-uri not defined properly"); }

		if (!config["headers"].IsMap()) {
			throw config_error("Headers are not a map");
		}

		for (auto entry: config["headers"]) {
			if (!entry.first.IsScalar()) {
				throw config_error("A header key is not scalar");
			}

			if (entry.second.IsSequence()) {
				for (auto value: entry.second) {
					if (!value.IsScalar()) {
						throw config_error("A header value is not scalar");
					}

					headers_.insert(std::make_pair(
						entry.first.as<std::string>(),
						value.as<std::string>()
					));
				}
			} else if (entry.second.IsScalar()){
				headers_.insert(std::make_pair(
					entry.first.as<std::string>(),
					entry.second.as<std::string>()
				));
			} else {
				throw config_error("A header value is not scalar");
			}
		}

		// load worker-id
		if (config["worker-id"] && config["worker-id"].IsScalar()) {
			worker_id_ = config["worker-id"].as<size_t>();
		} else { throw config_error("Item worker-id not defined properly"); }

		// load file-manager
		if (config["file-manager"] && config["file-manager"].IsMap()) {
			auto fileman = config["file-manager"];
			if (fileman["file-collector"] && fileman["file-collector"].IsMap()) {
				if (fileman["file-collector"]["hostname"] && fileman["file-collector"]["hostname"].IsScalar()) {
					fileman_config_.remote_url = fileman["file-collector"]["hostname"].as<std::string>();
				} // no throw... can be omitted
				if (fileman["file-collector"]["username"] && fileman["file-collector"]["username"].IsScalar()) {
					fileman_config_.username = fileman["file-collector"]["username"].as<std::string>();
				} // no throw... can be omitted
				if (fileman["file-collector"]["password"] && fileman["file-collector"]["password"].IsScalar()) {
					fileman_config_.password = fileman["file-collector"]["password"].as<std::string>();
				} // no throw... can be omitted
			} // no throw... can be omitted

			if (fileman["cache"] && fileman["cache"].IsMap()) {
				if (fileman["cache"]["cache-dir"] && fileman["cache"]["cache-dir"].IsScalar()) {
					fileman_config_.cache_dir = fileman["cache"]["cache-dir"].as<std::string>();
				} // no throw... can be omitted
			} // no throw... can be omitted
		} // no throw... can be omitted

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
				log_config_.log_file_size = config["logger"]["max-size"].as<size_t>();
			} // no throw... can be omitted
			if (config["logger"]["rotations"] && config["logger"]["rotations"].IsScalar()) {
				log_config_.log_files_count = config["logger"]["rotations"].as<size_t>();
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
				limits_.stack_size = limits["stack-size"].as<size_t>();
			} // no throw... can be omitted
			if (limits["memory"] && limits["memory"].IsScalar()) {
				limits_.memory_usage = limits["memory"].as<size_t>();
			} // no throw... can be omitted
			if (limits["parallel"] && limits["parallel"].IsScalar()) {
				// = limits["parallel"].as<bool>(); // TODO
			} // no throw... can be omitted
			if (limits["disk-blocks"] && limits["disk-blocks"].IsScalar()) {
				limits_.disk_blocks = limits["disk-blocks"].as<size_t>();
			} // no throw... can be omitted
			if (limits["disk-inodes"] && limits["disk-inodes"].IsScalar()) {
				limits_.disk_inodes = limits["disk-inodes"].as<size_t>();
			} // no throw... can be omitted
		} // no throw... can be omitted

		// sandboxes wrappers limits
		if (config["sandboxes-wrap-limits"] && config["sandboxes-wrap-limits"].IsSequence()) {
			for (auto &wraplim : config["sandboxes-wrap-limits"]) {
				if (wraplim["name"] && wraplim["name"].IsScalar() &&
						wraplim["time"] && wraplim["time"].IsScalar()) {
					sandboxes_limits_.insert(std::make_pair(wraplim["name"].as<std::string>(),
											 wraplim["time"].as<size_t>()));
				} // no throw... can be omitted
			}
		} // no throw... can be omitted

		if (config["bound-directories"] && config["bound-directories"].IsMap()) {
			for (auto &dir : config["bound-directories"]) {
				if (!dir.first.IsScalar()) {
					throw config_error("A bound directory alias is not scalar");
				}

				if (!dir.second.IsScalar()) {
					throw config_error("A bound directory path is not scalar");
				}

				limits_.bound_dirs.emplace(
					dir.first.as<std::string>(),
					dir.second.as<std::string>()
				);
			}
		}

	} catch (YAML::Exception &ex) {
		throw config_error("Default worker configuration was not loaded: " + std::string(ex.what()));
	}
}

size_t worker_config::get_worker_id()
{
	return worker_id_;
}

std::string worker_config::get_broker_uri() const
{
	return broker_uri_;
}

const worker_config::header_map_t &worker_config::get_headers() const
{
	return headers_;
}

const log_config &worker_config::get_log_config()
{
	return log_config_;
}

const fileman_config &worker_config::get_fileman_config()
{
	return fileman_config_;
}

const sandbox_limits &worker_config::get_limits()
{
	return limits_;
}

const std::map<std::string, size_t> &worker_config::get_sandboxes_limits()
{
	return sandboxes_limits_;
}

#ifndef CODEX_WORKER_WORKER_CONFIG_H
#define CODEX_WORKER_WORKER_CONFIG_H


#include <iostream>
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "log_config.h"
#include "fileman_config.h"
#include "../sandbox/sandbox_base.h"


/**
 * An object representation of a worker's configuration
 */
class worker_config {
public:

	/** Type of the header map */
	typedef std::multimap<std::string, std::string> header_map_t;

	/**
	 * The default constructor
	 */
	worker_config ();

	/**
	 * A constructor that loads the configuration from a YAML document
	 */
	worker_config (const YAML::Node &config);

	std::string get_worker_id();
	virtual std::string get_broker_uri() const;
	virtual const header_map_t &get_headers() const;
	const log_config &get_log_config();
	const fileman_config &get_fileman_config();
	const sandbox_limits &get_limits();
	const std::map<std::string, size_t> &get_sandboxes_limits();

private:

	std::string worker_id_;
	/** Broker URI */
	std::string broker_uri_;
	header_map_t headers_;
	log_config log_config_;
	fileman_config fileman_config_;
	sandbox_limits limits_;
	std::map<std::string, size_t> sandboxes_limits_;
};

class config_error : public std::runtime_error {
public:
	explicit config_error (const std::string &msg): std::runtime_error(msg)
	{
	}
};

#endif //CODEX_WORKER_WORKER_CONFIG_H

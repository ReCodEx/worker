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
 * An object representation of a default worker's configuration
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

	/**
	 * Get worker ID which has to be unique at least in context of one machine.
	 * @return not integer but textual description for better debuggin and human readibility
	 */
	std::string get_worker_id();
	/**
	 * Defines address on which broker run.
	 * @return
	 */
	virtual std::string get_broker_uri() const;
	/**
	 * Headers defined in configuration file, which will be sent to broker.
	 * @return
	 */
	virtual const header_map_t &get_headers() const;
	/**
	 * Get wrapper for logger configuration.
	 * @return constant reference to log_config structure
	 */
	const log_config &get_log_config();
	/**
	 * Get wrapper for file manager configuration.
	 * @return constant reference to fileman_config structure
	 */
	const fileman_config &get_fileman_config();
	/**
	 * Get default worker sandbox limits. Which will be used as defaults if not defined in job configuration.
	 * @return non editable reference to sandbox_limits structure
	 */
	const sandbox_limits &get_limits();
	/**
	 * Return limits which are applied for sandbox wrapper classes.
	 * Its a security restriction if sandbox gets mad and unresponsive.
	 * @return associative array indexed with sandbox name and with values of sandbox wrapper time limit
	 */
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


/**
 * Default worker configuration exception.
 */
class config_error : public std::runtime_error {
public:
	/**
	 * Construction with message returned with @ref what() method.
	 * @param msg description of exception circumstances
	 */
	explicit config_error (const std::string &msg) : std::runtime_error(msg)
	{
	}
};

#endif //CODEX_WORKER_WORKER_CONFIG_H

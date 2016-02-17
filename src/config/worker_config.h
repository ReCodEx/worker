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
class worker_config
{
public:
	/** Type of the header map */
	typedef std::multimap<std::string, std::string> header_map_t;

	/**
	 * The default constructor
	 */
	worker_config();

	/**
	 * A constructor that loads the configuration from a YAML document
	 */
	worker_config(const YAML::Node &config);

	/**
	 * Get worker ID which has to be unique at least in context of one machine.
	 * @return not integer but textual description for better debuggin and human readibility
	 */
	size_t get_worker_id();
	/**
	 * Working directory path defined in config file.
	 * @return
	 */
	std::string get_working_directory();
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
	 * Get path to the caching directory
	 */
	std::string get_cache_dir() const;

	/**
	 * Get wrapper for logger configuration.
	 * @return constant reference to log_config structure
	 */
	const log_config &get_log_config();
	/**
	 * Get wrapper for file manager configuration.
	 * @return constant reference to fileman_config structure
	 */
	const std::vector<fileman_config> &get_filemans_configs();
	/**
	 * Get default worker sandbox limits. Which will be used as defaults if not defined in job configuration.
	 * @return non editable reference to sandbox_limits structure
	 */
	const sandbox_limits &get_limits();

private:
	/** Unique worker number in context of one machine (0-100) */
	size_t worker_id_;
	/** Working directory of whole server */
	std::string working_directory_;
	/** Broker URI, address where broker is listening */
	std::string broker_uri_;
	/** Header which are sent to broker and should specify worker abilities */
	header_map_t headers_;
	/** The caching directory path */
	std::string cache_dir_;
	/** Configuration of logger */
	log_config log_config_;
	/** Default configuration of file managers */
	std::vector<fileman_config> filemans_configs_;
	/** Default sandbox limits */
	sandbox_limits limits_;
};


/**
 * Default worker configuration exception.
 */
class config_error : public std::runtime_error
{
public:
	/**
	 * Construction with message returned with @ref what() method.
	 * @param msg description of exception circumstances
	 */
	explicit config_error(const std::string &msg) : std::runtime_error(msg)
	{
	}
};

#endif // CODEX_WORKER_WORKER_CONFIG_H

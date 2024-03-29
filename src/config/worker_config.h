#ifndef RECODEX_WORKER_WORKER_CONFIG_H
#define RECODEX_WORKER_WORKER_CONFIG_H


#include <iostream>
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>
#include <filesystem>

#include "log_config.h"
#include "fileman_config.h"
#include "sandbox/sandbox_base.h"

namespace fs = std::filesystem;

/**
 * An object representation of a default worker's configuration
 */
class worker_config
{
public:
	/** Type of the header map */
	using header_map_t = std::multimap<std::string, std::string>;

	/**
	 * The default constructor
	 */
	worker_config();

	/**
	 * A constructor that loads the configuration from a YAML document
	 * @param config YAML representation of worker configuration
	 */
	worker_config(const YAML::Node &config);

	/**
	 * Virtual destructor to avoid memory leaks when dealocating childs.
	 */
	virtual ~worker_config();

	/**
	 * Get worker ID which has to be unique at least in context of one machine.
	 * @return integer which can be used also as identifier/index of sandbox
	 */
	virtual std::size_t get_worker_id() const;
	/**
	 * Get worker human readable description (name), which will be shown in broker logs.
	 * @return string with the description
	 */
	virtual const std::string &get_worker_description() const;
	/**
	 * Working directory path defined in config file.
	 * Basically directory which is used as central point of work, all things should be done here.
	 * @return textual representation of path
	 */
	virtual const std::string &get_working_directory() const;
	/**
	 * Defines address on which broker run.
	 * @return textual representation of address or domain name and port
	 */
	virtual const std::string &get_broker_uri() const;
	/**
	 * Headers defined in configuration file, which will be sent to broker.
	 * @return associative array
	 */
	virtual const header_map_t &get_headers() const;
	/**
	 * Gets hwgroup string description.
	 * @return hardware group of this worker
	 */
	virtual const std::string &get_hwgroup() const;

	/**
	 * Get the maximum number of pings in a row without response before the broker is considered disconnected.
	 * @return broker liveness integer
	 */
	virtual std::size_t get_max_broker_liveness() const;

	/**
	 * Get the interval between pings sent to the broker.
	 * @return milliseconds representation from std
	 */
	virtual std::chrono::milliseconds get_broker_ping_interval() const;

	/**
	 * Get path to the caching directory.
	 * @return textual representation of path
	 */
	virtual const std::string &get_cache_dir() const;

	/**
	 * Get wrapper for logger configuration.
	 * @return constant reference to log_config structure
	 */
	virtual const log_config &get_log_config() const;
	/**
	 * Get wrapper for file manager configuration.
	 * @return constant reference to fileman_config structure
	 */
	virtual const std::vector<fileman_config> &get_filemans_configs() const;
	/**
	 * Get default worker sandbox limits. Which will be used as defaults if not defined in job configuration.
	 * @return non editable reference to sandbox_limits structure
	 */
	virtual const sandbox_limits &get_limits() const;

	/**
	 * Get maximal length of output which can be written to the results.
	 * @return length of output in bytes
	 */
	virtual std::size_t get_max_output_length() const;

	/**
	 * Get maximal length of output which can be copied into results folder.
	 * @return length of output in bytes
	 */
	virtual std::size_t get_max_carboncopy_length() const;

	/**
	 * Get flag which determines if cleanup is made after sumbission is evaluated.
	 * @return
	 */
	virtual bool get_cleanup_submission() const;

private:
	/** Unique worker number in context of one machine (0-100 preferably) */
	std::size_t worker_id_ = 0;
	/** Human readable description of the worker for logging purposes */
	std::string worker_description_ = "";
	/** Working directory of whole worker used as base directory for all temporary files */
	std::string working_directory_ = "";
	/** Broker URI, address where broker is listening */
	std::string broker_uri_ = "";
	/** Header which are sent to broker and should specify worker abilities */
	header_map_t headers_ = {};
	/** Hwgroup which is sent to broker and is used in job configuration to select right limits */
	std::string hwgroup_ = {};
	/** Maximum number of pings in a row without response before the broker is considered disconnected */
	std::size_t max_broker_liveness_ = 4;
	/** How often should the worker ping the broker */
	std::chrono::milliseconds broker_ping_interval_ = std::chrono::milliseconds(1000);
	/** The caching directory path */
	std::string cache_dir_ = "";
	/** Configuration of logger */
	log_config log_config_ = {};
	/** Default configuration of file managers */
	std::vector<fileman_config> filemans_configs_ = {};
	/** Default sandbox limits */
	sandbox_limits limits_ = {};
	/** Maximal length of output from sandbox which can be written to the results file, in bytes. */
	std::size_t max_output_length_ = 0;
	/** Maximal lenght of output from sandbox which can be copied into results folder, in bytes */
	std::size_t max_carboncopy_length_ = 0;
	/** If true then all files created during evaluation of job will be deleted at the end. */
	bool cleanup_submission_ = true;
};


/**
 * Default worker configuration exception.
 */
class config_error : public std::runtime_error
{
public:
	/**
	 * Construction with message returned with @a what() method.
	 * @param msg description of exception circumstances
	 */
	explicit config_error(const std::string &msg) : std::runtime_error(msg)
	{
	}
};

#endif // RECODEX_WORKER_WORKER_CONFIG_H

#ifndef CODEX_WORKER_WORKER_CONFIG_H
#define CODEX_WORKER_WORKER_CONFIG_H


#include <iostream>
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>

/**
 * An object representation of a worker's configuration
 */
class worker_config {
public:
	/** Type of the header map */
	typedef std::multimap<std::string, std::string> header_map_t;

private:
	/** URI of the task broker */
	std::string broker_uri = "";

	/** Headers that describe this worker's capabilities */
	header_map_t headers;

public:
	/**
	 * The default constructor
	 */
	worker_config ();

	/**
	 * A constructor that loads the configuration from a YAML document
	 */
	worker_config (const YAML::Node &config);

	/**
	 * Get the URI of the task broker
	 * @return A ZeroMQ compatible URI
	 */
	virtual std::string get_broker_uri () const;

	/**
	 * Get the headers this worker uses to describe itself to the broker
	 */
	virtual const header_map_t &get_headers () const;
};

class config_error : public std::runtime_error {
public:
	explicit config_error (const std::string &msg): std::runtime_error(msg)
	{
	}
};

#endif //CODEX_WORKER_WORKER_CONFIG_H

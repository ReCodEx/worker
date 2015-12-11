#ifndef CODEX_WORKER_WORKER_CONFIG_H
#define CODEX_WORKER_WORKER_CONFIG_H


#include <iostream>
#include <string>
#include <map>

/**
 * An object representation of a worker's configuration
 */
class worker_config {
public:

	/**
	 * The default constructor
	 */
	worker_config ();

	/**
	 * A constructor that loads the configuration from a file
	 */
	worker_config (std::istream &input);

	/**
	 * Get the URI of the task broker
	 * @return A ZeroMQ compatible URI
	 */
	virtual std::string get_broker_uri () const;

	/**
	 * The type of the header map
	 */
	typedef std::multimap<std::string, std::string> header_map_t;

	/**
	 * Get the headers this worker uses to describe itself to the broker
	 */
	virtual const header_map_t &get_headers () const;
private:
	header_map_t headers;
};


#endif //CODEX_WORKER_WORKER_CONFIG_H

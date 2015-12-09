#ifndef CODEX_WORKER_WORKER_CONFIG_H
#define CODEX_WORKER_WORKER_CONFIG_H


#include <iostream>
#include <string>

/**
 * An object representation of a worker's configuration
 */
class worker_config {
public:
	/**
	 * A constructor that loads the configuration from a file
	 */
	worker_config (std::istream &input);

	/**
	 * Get the URI of the task broker
	 * @return A ZeroMQ compatible URI
	 */
	std::string get_broker_uri () const;
};


#endif //CODEX_WORKER_WORKER_CONFIG_H

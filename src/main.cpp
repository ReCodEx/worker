#include <iostream>
#include <yaml-cpp/yaml.h>

#include "worker_config.hpp"
#include "broker_connection.hpp"
#include "connection_proxy.hpp"

struct receive_task {
	bool operator() ()
	{
		std::cout << "Task received" << std::endl;
		return true;
	}
};

int main (int argc, char **argv)
{
	YAML::Node yaml;

	try {
		yaml = YAML::LoadFile("config.yml");
	} catch (YAML::Exception) {
		std::cerr << "Error loading config file" << std::endl;
		return 1;
	}

	worker_config config(yaml);
	connection_proxy proxy;

	broker_connection <connection_proxy, receive_task> connection(config, &proxy);
	connection.connect();

	while (true) {
		connection.receive_task();
	}

	return 0;
}
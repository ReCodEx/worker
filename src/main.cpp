#include <iostream>

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
	worker_config config;
	connection_proxy proxy;

	broker_connection <connection_proxy, receive_task> connection(config, &proxy);
	connection.connect();

	while (true) {
		connection.receive_task();
	}

	return 0;
}
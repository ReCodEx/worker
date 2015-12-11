#include <iostream>

#include "worker_config.hpp"
#include "broker_connection.hpp"

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
	broker_connection connection(config);

	connection.receive_tasks<receive_task>();

	return 0;
}
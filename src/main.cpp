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
	worker_config config(std::cin);
	broker_connection connection(config);
	broker_connection::header_map_t headers;

	headers.insert(std::make_pair("env", "c"));
	headers.insert(std::make_pair("env", "python"));
	headers.insert(std::make_pair("threads", "2"));

	connection.receive_tasks<receive_task>(headers);

	return 0;
}
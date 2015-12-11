#include "worker_config.hpp"

worker_config::worker_config (std::istream &input)
{

}

std::string worker_config::get_broker_uri () const
{
	return "tcp://localhost:9876";
}

worker_config::worker_config ()
{
	headers.insert(std::make_pair("env", "c"));
	headers.insert(std::make_pair("env", "python"));
	headers.insert(std::make_pair("threads", "2"));
}

const worker_config::header_map_t &worker_config::get_header_map () const
{
	return headers;
}

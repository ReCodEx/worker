#include "worker_config.hpp"

worker_config::worker_config (std::istream &input)
{

}

std::string worker_config::get_broker_uri () const
{
	return "tcp://localhost:9876";
}


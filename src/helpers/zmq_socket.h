#ifndef CODEX_HELPERS_ZMQ_SOCKET_H
#define CODEX_HELPERS_ZMQ_SOCKET_H

#include <string>
#include <vector>
#include <zmq.hpp>

namespace helpers
{

	bool send_through_socket(zmq::socket_t &socket, const std::vector<std::string> &msg);
	bool recv_from_socket(zmq::socket_t &socket, std::vector<std::string> &target, bool *terminate = nullptr);
}


#endif // CODEX_HELPERS_ZMQ_SOCKET_H

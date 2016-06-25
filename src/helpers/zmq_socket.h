#ifndef RECODEX_HELPERS_ZMQ_SOCKET_H
#define RECODEX_HELPERS_ZMQ_SOCKET_H

#include <string>
#include <vector>
#include <zmq.hpp>

namespace helpers
{
	/**
	 * Sends multipart message through given zmq socket.
	 * @param socket socket to which messages will be sent
	 * @param msg multipart message which will be sent
	 * @return true on success, false otherwise.
	 */
	bool send_through_socket(zmq::socket_t &socket, const std::vector<std::string> &msg);
	/**
	 * Receive multipart message from given zmq socket.
	 * @param socket messages should be received here
	 * @param target reference to modifiable vector, after calling it should contain received messages
	 * @param terminate pointer to boolean variable which will be set to true if exception occured on receiving
	 * @return true on success, false otherwise
	 */
	bool recv_from_socket(zmq::socket_t &socket, std::vector<std::string> &target, bool *terminate = nullptr);
}

#endif // RECODEX_HELPERS_ZMQ_SOCKET_H

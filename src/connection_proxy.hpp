#ifndef CODEX_WORKER_CONNECTION_PROXY_HPP
#define CODEX_WORKER_CONNECTION_PROXY_HPP

#include <iostream>
#include <zmq.hpp>

/**
 * A trivial wrapper for the ZeroMQ dealer socket used by broker_connection
 * The purpose of this class is to facilitate testing of the broker_connection class
 */
class connection_proxy {
private:
	zmq::context_t context;
	zmq::socket_t socket;

public:
	connection_proxy () : context(1), socket(context, ZMQ_DEALER)
	{
	}

	void connect (const std::string &addr)
	{
		return socket.connect(addr);
	}

	/**
	 * Send data through the socket
	 */
	size_t send (const void *data, size_t size, int flags)
	{
		return socket.send(data, size, flags);
	}

	/**
	 * Receive data from the socket
	 */
	bool recv (zmq::message_t &msg)
	{
		return socket.recv(&msg);
	}
};

#endif //CODEX_WORKER_CONNECTION_PROXY_HPP

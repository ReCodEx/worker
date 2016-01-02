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
		socket.connect(addr);
	}

	/**
	 * Send data through the socket
	 */
	bool send (const std::vector<std::string> &msg)
	{
		for (auto it = std::begin(msg); it != std::end(msg); ++it) {
			bool retval = (bool) socket.send(
				it->c_str(),
				it->size(),
				std::next(it) != std::end(msg) ? ZMQ_SNDMORE : 0
			);

			if (!retval) {
				return false;
			}
		}

		return true;
	}

	/**
	 * Receive data from the socket
	 */
	bool recv (std::vector<std::string> &target, bool *terminate = nullptr)
	{
		zmq::message_t msg;
		target.clear();

		do {
			bool retval;

			try {
				retval = socket.recv(&msg);
			} catch (zmq::error_t) {
				if (terminate != nullptr) {
					*terminate = true;
					retval = false;
				}
			}

			if (!retval) {
				return false;
			}

			target.push_back(std::string(static_cast<char *>(msg.data()), msg.size()));
		} while (msg.more());

		return true;
	}
};

#endif //CODEX_WORKER_CONNECTION_PROXY_HPP

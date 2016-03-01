#include "zmq_socket.h"

bool helpers::send_through_socket(zmq::socket_t &socket, const std::vector<std::string> &msg)
{
	for (auto it = std::begin(msg); it != std::end(msg); ++it) {
		bool retval = socket.send(it->c_str(), it->size(), std::next(it) != std::end(msg) ? ZMQ_SNDMORE : 0) >= 0;

		if (!retval) {
			return false;
		}
	}

	return true;
}

bool helpers::recv_from_socket(zmq::socket_t &socket, std::vector<std::string> &target, bool *terminate)
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
			}
			retval = false;
		}

		if (!retval) {
			return false;
		}

		target.emplace_back(static_cast<char *>(msg.data()), msg.size());
	} while (msg.more());

	return true;
}

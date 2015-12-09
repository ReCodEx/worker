#ifndef CODEX_WORKER_BROKER_CONNECTION_H
#define CODEX_WORKER_BROKER_CONNECTION_H


#include <zmq.hpp>
#include <map>

#include "worker_config.hpp"

class broker_connection {
public:
	typedef std::multimap<std::string, std::string> header_map_t;
private:
	const worker_config &config;
	zmq::context_t context;
	zmq::socket_t socket;

	/**
	 * Send the INIT command to the broker.
	 * @param headers A map that specifies which tasks this worker accepts
	 */
	void command_init (const header_map_t &headers)
	{
		socket.send("init", 4, ZMQ_SNDMORE);

		auto &last = --headers.end();

		for (auto it = headers.begin(); it != headers.end(); ++it) {
			auto pair = *it;

			char buf[256];
			int count = snprintf(buf, sizeof(buf), "%s=%s", pair.first, pair.second);

			if (count < 0) {
				throw;
			}

			socket.send(buf, (size_t) count, it == --headers.end() ? 0 : ZMQ_SNDMORE);
		}
	}

	/**
	 * Send the ACK command to the broker
	 */
	void command_ack ()
	{

	}

public:
	broker_connection (const worker_config &config) :
		context(1), socket(context, ZMQ_DEALER), config(config)
	{
	}

	/**
	 * Initialize the connection with the broker and receive tasks.
	 * Blocks execution until interrupted.
	 *
	 * @param headers A map that specifies which tasks this worker accepts
	 */
	template <typename task_callback>
	void receive_tasks (const header_map_t &headers)
	{
		task_callback cb;

		socket.connect(config.get_broker_uri());
		command_init(headers);

		while (true) {
			zmq::message_t request;
			socket.recv(&request);
			cb();
		}
	}
};

#endif //CODEX_WORKER_BROKER_CONNECTION_H

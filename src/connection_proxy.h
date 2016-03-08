#ifndef CODEX_WORKER_CONNECTION_PROXY_HPP
#define CODEX_WORKER_CONNECTION_PROXY_HPP

#include <iostream>
#include <memory>
#include <zmq.hpp>
#include <string>

#include "broker_connection.h"
#include "helpers/zmq_socket.h"

static const std::string JOB_SOCKET_ID = "jobs";

/**
 * A trivial wrapper for the ZeroMQ dealer socket used by broker_connection
 * The purpose of this class is to facilitate testing of the broker_connection class
 */
class connection_proxy
{
private:
	zmq::socket_t broker_;
	zmq::socket_t jobs_;
	zmq::pollitem_t items_[2];
	std::shared_ptr<zmq::context_t> context_;

public:
	/**
	 * TODO: documentation
	 * @param context
	 */
	connection_proxy(std::shared_ptr<zmq::context_t> context)
		: broker_(*context, ZMQ_DEALER), jobs_(*context, ZMQ_PAIR), context_(context)
	{
		items_[0].socket = (void *) broker_;
		items_[0].fd = 0;
		items_[0].events = ZMQ_POLLIN;
		items_[0].revents = 0;

		items_[1].socket = (void *) jobs_;
		items_[1].fd = 0;
		items_[1].events = ZMQ_POLLIN;
		items_[1].revents = 0;
	}

	/**
	 * TODO: documentation
	 * @param addr
	 */
	void connect(const std::string &addr)
	{
		broker_.setsockopt(ZMQ_LINGER, 0);
		broker_.connect(addr);
		jobs_.bind("inproc://" + JOB_SOCKET_ID);
	}

	/**
	 * Disconnect the broker socket and connect again
	 */
	void reconnect_broker(const std::string &addr)
	{
		broker_.close();
		broker_ = zmq::socket_t(*context_, ZMQ_DEALER);
		broker_.setsockopt(ZMQ_LINGER, 0);
		broker_.connect(addr);
	}

	/**
	 * Block execution until a message arrives to a socket
	 */
	void poll(message_origin::set &result,
		std::chrono::milliseconds timeout,
		bool &terminate,
		std::chrono::milliseconds &elapsed_time)
	{
		result.reset();

		try {
			auto time_before_poll = std::chrono::system_clock::now();
			zmq::poll(items_, 2, timeout.count());
			auto time_after_poll = std::chrono::system_clock::now();

			elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(time_after_poll - time_before_poll);
		} catch (zmq::error_t) {
			terminate = true;
			return;
		}

		if (items_[0].revents & ZMQ_POLLIN) {
			result.set(message_origin::BROKER, true);
		}

		if (items_[1].revents & ZMQ_POLLIN) {
			result.set(message_origin::JOBS, true);
		}
	}

	/**
	 * Send data to the broker
	 */
	bool send_broker(const std::vector<std::string> &msg)
	{
		return helpers::send_through_socket(broker_, msg);
	}

	/**
	 * Send data through the job socket
	 */
	bool send_jobs(const std::vector<std::string> &msg)
	{
		return helpers::send_through_socket(jobs_, msg);
	}

	/**
	 * Receive data from the broker
	 */
	bool recv_broker(std::vector<std::string> &target, bool *terminate = nullptr)
	{
		return helpers::recv_from_socket(broker_, target, terminate);
	}

	/**
	 * Receive data from the job socket
	 */
	bool recv_jobs(std::vector<std::string> &target, bool *terminate = nullptr)
	{
		return helpers::recv_from_socket(jobs_, target, terminate);
	}
};

#endif // CODEX_WORKER_CONNECTION_PROXY_HPP

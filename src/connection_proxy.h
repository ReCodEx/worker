#ifndef RECODEX_WORKER_CONNECTION_PROXY_HPP
#define RECODEX_WORKER_CONNECTION_PROXY_HPP

#include <iostream>
#include <memory>
#include <zmq.hpp>
#include <string>

#include "broker_connection.h"
#include "helpers/zmq_socket.h"

static const std::string JOB_SOCKET_ID = "jobs";
static const std::string PROGRESS_SOCKET_ID = "progress";

/**
 * A trivial wrapper for the ZeroMQ dealer socket used by broker_connection
 * The purpose of this class is to facilitate testing of the broker_connection class
 */
class connection_proxy
{
private:
	static const size_t socket_count_ = 3;
	zmq::socket_t broker_;
	zmq::socket_t jobs_;
	zmq::socket_t progress_;
	zmq::pollitem_t items_[socket_count_];
	std::shared_ptr<zmq::context_t> context_;

public:
	/**
	 * @param context a ZeroMQ context
	 */
	connection_proxy(std::shared_ptr<zmq::context_t> context)
		: broker_(*context, ZMQ_DEALER), jobs_(*context, ZMQ_PAIR), progress_(*context, ZMQ_PAIR), context_(context)
	{
		items_[0].socket = (void *) broker_;
		items_[0].fd = 0;
		items_[0].events = ZMQ_POLLIN;
		items_[0].revents = 0;

		items_[1].socket = (void *) jobs_;
		items_[1].fd = 0;
		items_[1].events = ZMQ_POLLIN;
		items_[1].revents = 0;

		items_[2].socket = (void *) progress_;
		items_[2].fd = 0;
		items_[2].events = ZMQ_POLLIN;
		items_[2].revents = 0;
	}

	/**
	 * Connect and bind all the necessary sockets
	 * @param addr address of the broker
	 */
	void connect(const std::string &addr)
	{
		broker_.setsockopt(ZMQ_LINGER, 0);
		broker_.connect(addr);
		jobs_.bind("inproc://" + JOB_SOCKET_ID);
		progress_.bind("inproc://" + PROGRESS_SOCKET_ID);
	}

	/**
	 * Disconnect the broker socket and connect again.
	 * This results in dropping unsent messages.
	 * @param addr address of the broker
	 */
	void reconnect_broker(const std::string &addr)
	{
		broker_.disconnect(addr);
		broker_.connect(addr);
	}

	/**
	 * Block execution until a message arrives to a socket
	 * @param result set by the method to contain sockets that received a message
	 * @param timeout the maximum time the method should wait for messages
	 * @param terminate set to true if the underlying ZeroMQ sockets can't receive messages anymore
	 * @param elapsed_time set to the time spent in the method
	 */
	void poll(message_origin::set &result,
		std::chrono::milliseconds timeout,
		bool &terminate,
		std::chrono::milliseconds &elapsed_time)
	{
		result.reset();

		try {
			auto time_before_poll = std::chrono::system_clock::now();
			zmq::poll(items_, socket_count_, (long) timeout.count());
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

		if (items_[2].revents & ZMQ_POLLIN) {
			result.set(message_origin::PROGRESS, true);
		}
	}

	/**
	 * Send data to the broker
	 * @param msg message to be sent
	 */
	bool send_broker(const std::vector<std::string> &msg)
	{
		return helpers::send_through_socket(broker_, msg);
	}

	/**
	 * Send data through the job socket
	 * @param msg message to be sent
	 */
	bool send_jobs(const std::vector<std::string> &msg)
	{
		return helpers::send_through_socket(jobs_, msg);
	}

	/**
	 * Receive data from the broker.
	 * This method should only be called after a successful poll call (only poll measures elapsed time).
	 * @param target where the received message should be stored
	 * @param terminate set to true if the underlying ZeroMQ sockets can't receive messages anymore
	 */
	bool recv_broker(std::vector<std::string> &target, bool *terminate = nullptr)
	{
		return helpers::recv_from_socket(broker_, target, terminate);
	}

	/**
	 * Receive data from the job socket.
	 * This method should only be called after a successful poll call (only poll measures elapsed time).
	 * @param target where the received message should be stored
	 * @param terminate set to true if the underlying ZeroMQ sockets can't receive messages anymore
	 */
	bool recv_jobs(std::vector<std::string> &target, bool *terminate = nullptr)
	{
		return helpers::recv_from_socket(jobs_, target, terminate);
	}

	/**
	 * Receive data from the progress socket (sent by the job evaluator).
	 * This method should only be called after a successful poll call (only poll measures elapsed time).
	 * @param target where the received message should be stored
	 * @param terminate set to true if the underlying ZeroMQ sockets can't receive messages anymore
	 */
	bool recv_progress(std::vector<std::string> &target, bool *terminate = nullptr)
	{
		return helpers::recv_from_socket(progress_, target, terminate);
	}
};

#endif // RECODEX_WORKER_CONNECTION_PROXY_HPP

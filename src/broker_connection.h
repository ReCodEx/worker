#ifndef CODEX_WORKER_BROKER_CONNECTION_H
#define CODEX_WORKER_BROKER_CONNECTION_H

#include <zmq.hpp>
#include <map>
#include <memory>
#include <bitset>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "config/worker_config.h"

/**
 * Contains types used by the proxy for polling
 */
struct message_origin {
	enum type { BROKER = 0, JOBS = 1 };

	typedef std::bitset<2> set;
};

/**
 * Represents a connection to the ReCodEx broker
 * When a job is received from the broker, a job callback is invoked to
 * process it.
 */
template <typename proxy> class broker_connection
{
private:
	const worker_config &config;
	std::shared_ptr<proxy> socket;
	std::shared_ptr<spdlog::logger> logger_;

public:
	broker_connection(
		const worker_config &config, std::shared_ptr<proxy> socket, std::shared_ptr<spdlog::logger> logger = nullptr)
		: config(config), socket(socket)
	{
		if (logger != nullptr) {
			logger_ = logger;
		} else {
			// Create logger manually to avoid global registration of logger
			auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
			logger_ = std::make_shared<spdlog::logger>("cache_manager_nolog", sink);
		}
	}

	/**
	 * Send the INIT command to the broker.
	 */
	void connect()
	{
		const worker_config::header_map_t &headers = config.get_headers();

		logger_->debug() << "Connecting to " << config.get_broker_uri();
		socket->connect(config.get_broker_uri());

		std::vector<std::string> msg = {"init"};

		for (auto &it : headers) {
			msg.push_back(it.first + "=" + it.second);
		}

		socket->send_broker(msg);
	}

	/**
	 * Receive and process tasks
	 * Blocks execution until the underlying ZeroMQ context is terminated
	 */
	void receive_tasks()
	{
		while (true) {
			std::vector<std::string> msg;
			message_origin::set result;
			bool terminate = false;

			socket->poll(result, -1, &terminate);

			if (terminate) {
				break;
			}

			if (result.test(message_origin::BROKER)) {
				socket->recv_broker(msg, &terminate);

				if (terminate) {
					break;
				}

				if (msg.at(0) == "eval") {
					socket->send_jobs(msg);
				}
			}

			if (result.test(message_origin::JOBS)) {
				socket->recv_jobs(msg, &terminate);

				if (terminate) {
					break;
				}

				if (msg.at(0) == "eval_finished") {
					socket->send_broker(msg);
				}
			}
		}

		logger_->emerg() << "Terminating to receive messages.";
	}
};

#endif // CODEX_WORKER_BROKER_CONNECTION_H

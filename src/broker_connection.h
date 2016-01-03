#ifndef CODEX_WORKER_BROKER_CONNECTION_H
#define CODEX_WORKER_BROKER_CONNECTION_H


#include <map>
#include <memory>
#include <bitset>

#include "spdlog/spdlog.h"
#include "config/worker_config.h"

/**
 * A structure that contains the information received with a job request
 */
struct job_request {
	const std::string job_id;
	const std::string job_url;
	const std::string result_url;

	job_request (std::string job_id, std::string job_url, std::string result_url) :
		job_id(job_id), job_url(job_url), result_url(result_url)
	{
	}
};

/**
 * Contains types used by the proxy for polling
 */
struct message_origin {
	enum type {
		BROKER = 0,
		JOBS = 1
	};

	typedef std::bitset<2> set;
};

/**
 * Represents a connection to the ReCodEx broker
 * When a job is received from the broker, a job callback is invoked to
 * process it.
 */
template <typename proxy, typename job_callback>
class broker_connection {
private:
	const worker_config &config;
	std::shared_ptr<proxy> socket;
	std::shared_ptr<spdlog::logger> logger_;
	job_callback *cb_;

	void process_eval (const std::vector<std::string> &msg)
	{
		if (msg.size() < 4) {
			return;
		}

		job_request request(
			msg.at(1),
			msg.at(2),
			msg.at(3)
		);

		(*cb_)(request);
	}
public:
	broker_connection (
		const worker_config &config,
		std::shared_ptr<proxy> socket,
		job_callback *cb,
		std::shared_ptr<spdlog::logger> logger = nullptr
	) :
		config(config), socket(socket), cb_(cb)
	{
		if(logger != nullptr) {
			logger_ = logger;
		} else {
			//Create logger manually to avoid global registration of logger
			auto sink = std::make_shared<spdlog::sinks::stderr_sink_st>();
			logger_ = std::make_shared<spdlog::logger>("cache_manager_nolog", sink);
			//Set loglevel to 'off' cause no logging
			logger_->set_level(spdlog::level::off);
		}
	}

	/**
	 * Send the INIT command to the broker.
	 */
	void connect ()
	{
		const worker_config::header_map_t &headers = config.get_headers();

		logger_->debug() << "Connecting to " << config.get_broker_uri();
		socket->connect(config.get_broker_uri());

		std::vector<std::string> msg = {"init"};

		for (auto &it: headers) {
			msg.push_back(it.first + "=" + it.second);
		}

		socket->send_broker(msg);
	}

	/**
	 * Receive and process tasks
	 * Blocks execution until the underlying ZeroMQ context is terminated
	 */
	void receive_tasks ()
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
					process_eval(msg);
				}
			}

			if (result.test(message_origin::JOBS)) {
				socket->recv_jobs(msg, &terminate);

				if (terminate) {
					break;
				}
			}
		}

		logger_->emerg() << "Terminating to receive messages.";
	}
};

#endif //CODEX_WORKER_BROKER_CONNECTION_H

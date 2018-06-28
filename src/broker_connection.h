#ifndef RECODEX_WORKER_BROKER_CONNECTION_H
#define RECODEX_WORKER_BROKER_CONNECTION_H

#include <zmq.hpp>
#include <map>
#include <memory>
#include <bitset>

#include "helpers/logger.h"
#include "config/worker_config.h"
#include "commands/command_holder.h"
#include "commands/broker_commands.h"
#include "commands/jobs_server_commands.h"

/**
 * Contains types used by the proxy for polling
 */
struct message_origin {
	/**
	 * Associates numbers with possible origins of messages
	 */
	enum type { BROKER = 0, JOBS = 1, PROGRESS = 2 };

	/**
	 * A set of origins from which there are incoming messages
	 */
	using set = std::bitset<3>;
};

/**
 * Represents a connection to the ReCodEx broker
 * When a job is received from the broker, a job callback is invoked to
 * process it.
 */
template <typename proxy> class broker_connection
{
private:
	std::shared_ptr<const worker_config> config_;
	std::shared_ptr<proxy> socket_;
	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<command_holder<broker_connection_context<proxy>>> broker_cmds_;
	std::shared_ptr<command_holder<broker_connection_context<proxy>>> jobs_server_cmds_;
	std::chrono::seconds reconnect_delay = std::chrono::seconds(1);
	std::string current_job_;

	/**
	 * Send the init command to the broker
	 */
	void send_init() const
	{
		const worker_config::header_map_t &headers = config_->get_headers();
		std::vector<std::string> msg = {"init", config_->get_hwgroup()};

		for (auto &it : headers) { msg.push_back(it.first + "=" + it.second); }
		msg.push_back("");
		msg.push_back("description=" + config_->get_worker_description());
		if (!current_job_.empty()) { msg.push_back("current_job=" + current_job_); }

		socket_->send_broker(msg);
	}

	/**
	 * Reconnect to the broker and wait for a while before trying to contact it again
	 */
	void reconnect()
	{
		socket_->reconnect_broker(config_->get_broker_uri());
		logger_->info("Going to sleep for {} seconds", reconnect_delay.count());
		std::this_thread::sleep_for(reconnect_delay);

		std::chrono::seconds max_reconnect_delay(32);
		if (reconnect_delay < max_reconnect_delay) { reconnect_delay *= 2; }
	}

	/**
	 * Reset the reconnection delay to its initial value
	 */
	void reset_reconnect_delay()
	{
		reconnect_delay = std::chrono::seconds(1);
	}

public:
	/**
	 * @param config configuration of the worker
	 * @param socket a proxy of ZeroMQ communication channels
	 * @param logger a logging service
	 */
	broker_connection(std::shared_ptr<const worker_config> config,
		std::shared_ptr<proxy> socket,
		std::shared_ptr<spdlog::logger> logger = nullptr)
		: config_(config), socket_(socket), logger_(logger), current_job_("")
	{
		if (logger_ == nullptr) { logger_ = helpers::create_null_logger(); }

		// prepare dependent context for commands (in this class)
		broker_connection_context<proxy> dependent_context = {socket_, config_, current_job_};

		// init broker commands
		broker_cmds_ = std::make_shared<command_holder<broker_connection_context<proxy>>>(dependent_context, logger_);
		broker_cmds_->register_command("eval", broker_commands::process_eval<broker_connection_context<proxy>>);
		broker_cmds_->register_command("intro", broker_commands::process_intro<broker_connection_context<proxy>>);

		// init jobs server commands
		jobs_server_cmds_ =
			std::make_shared<command_holder<broker_connection_context<proxy>>>(dependent_context, logger_);
		jobs_server_cmds_->register_command(
			"done", jobs_server_commands::process_done<broker_connection_context<proxy>>);
	}

	/**
	 * Connect to the broker and send it the INIT command
	 */
	void connect()
	{
		logger_->info("Connecting to {}", config_->get_broker_uri());
		socket_->connect(config_->get_broker_uri());
		send_init();
	}

	/**
	 * Receive and process tasks
	 * Blocks execution until the underlying ZeroMQ context is terminated
	 */
	void receive_tasks()
	{
		const std::chrono::milliseconds ping_interval = config_->get_broker_ping_interval();
		std::chrono::milliseconds poll_limit = ping_interval;
		std::size_t broker_liveness = config_->get_max_broker_liveness();

		while (true) {
			std::vector<std::string> msg;
			message_origin::set result;
			std::chrono::milliseconds poll_duration;
			bool terminate = false;

			try {
				socket_->poll(result, poll_limit, terminate, poll_duration);

				if (poll_duration >= poll_limit) {
					socket_->send_broker(std::vector<std::string>{"ping"});
					poll_limit = ping_interval;

					broker_liveness -= 1;
					if (broker_liveness == 0) {
						logger_->info("Broker connection expired - trying to reconnect");
						reconnect();
						send_init();
						broker_liveness = config_->get_max_broker_liveness();
					}
				} else {
					poll_limit -= poll_duration;
				}

				if (terminate) { break; }

				if (result.test(message_origin::BROKER)) {
					broker_liveness = config_->get_max_broker_liveness();
					reset_reconnect_delay();

					socket_->recv_broker(msg, &terminate);

					if (terminate) { break; }

					if (msg.size() >= 2 && msg.at(0) == "eval") { current_job_ = msg.at(1); }

					broker_cmds_->call_function(msg.at(0), msg);
				}

				if (result.test(message_origin::JOBS)) {
					socket_->recv_jobs(msg, &terminate);

					if (terminate) { break; }

					if (msg.size() >= 1 && msg.at(0) == "done") { current_job_ = ""; }

					jobs_server_cmds_->call_function(msg.at(0), msg);
				}

				if (result.test(message_origin::PROGRESS)) {
					socket_->recv_progress(msg, &terminate);

					if (terminate) { break; }

					socket_->send_broker(msg);
				}
			} catch (std::exception &e) {
				logger_->error("Unexpected error while receiving tasks: {}", e.what());
			}
		}

		logger_->critical("Ceasing to receive messages.");
	}
};

#endif // RECODEX_WORKER_BROKER_CONNECTION_H

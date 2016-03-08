#ifndef CODEX_WORKER_BROKER_CONNECTION_H
#define CODEX_WORKER_BROKER_CONNECTION_H

#include <zmq.hpp>
#include <map>
#include <memory>
#include <bitset>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "config/worker_config.h"
#include "commands/command_holder.h"
#include "commands/broker_commands.h"
#include "commands/jobs_server_commands.h"

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
	std::shared_ptr<const worker_config> config_;
	std::shared_ptr<proxy> socket_;
	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<command_holder<broker_connection_context<proxy>>> broker_cmds_;
	std::shared_ptr<command_holder<broker_connection_context<proxy>>> jobs_server_cmds_;
	std::chrono::seconds reconnect_delay = std::chrono::seconds(1);

	/**
	 * Send the init command to the broker
	 */
	void send_init() const
	{
		const worker_config::header_map_t &headers = config_->get_headers();
		std::vector<std::string> msg = {"init"};

		for (auto &it : headers) {
			msg.push_back(it.first + "=" + it.second);
		}

		socket_->send_broker(msg);
	}

	/**
	 * Reconnect to the broker and wait for a while before trying to contact it again
	 */
	void reconnect()
	{
		socket_->reconnect_broker(config_->get_broker_uri());
		logger_->debug() << "Going to sleep for " + std::to_string(reconnect_delay.count()) + " seconds";
		std::this_thread::sleep_for(reconnect_delay);

		std::chrono::seconds max_reconnect_delay(32);
		if (reconnect_delay < max_reconnect_delay) {
			reconnect_delay *= 2;
		}
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
	 * TODO: documentation
	 * @param config
	 * @param socket
	 * @param logger
	 */
	broker_connection(std::shared_ptr<const worker_config> config,
		std::shared_ptr<proxy> socket,
		std::shared_ptr<spdlog::logger> logger = nullptr)
		: config_(config), socket_(socket)
	{
		if (logger != nullptr) {
			logger_ = logger;
		} else {
			// Create logger manually to avoid global registration of logger
			auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
			logger_ = std::make_shared<spdlog::logger>("cache_manager_nolog", sink);
		}

		// prepare dependent context for commands (in this class)
		broker_connection_context<proxy> dependent_context = {socket_, config_};

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
		logger_->debug() << "Connecting to " << config_->get_broker_uri();
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
		size_t broker_liveness = config_->get_max_broker_liveness();

		while (true) {
			std::vector<std::string> msg;
			message_origin::set result;
			std::chrono::milliseconds poll_duration;
			bool terminate = false;

			socket_->poll(result, poll_limit, terminate, poll_duration);

			if (poll_duration >= poll_limit) {
				logger_->debug() << "Sending a ping";
				socket_->send_broker(std::vector<std::string>{"ping"});
				poll_limit = ping_interval;

				broker_liveness -= 1;
				if (broker_liveness == 0) {
					logger_->info() << "Broker connection expired - trying to reconnect";
					reconnect();
					send_init();
					broker_liveness = config_->get_max_broker_liveness();
				}
			} else {
				poll_limit -= poll_duration;
			}

			if (terminate) {
				break;
			}

			if (result.test(message_origin::BROKER)) {
				broker_liveness = config_->get_max_broker_liveness();
				reset_reconnect_delay();

				socket_->recv_broker(msg, &terminate);

				if (terminate) {
					break;
				}

				broker_cmds_->call_function(msg.at(0), msg);
			}

			if (result.test(message_origin::JOBS)) {
				socket_->recv_jobs(msg, &terminate);

				if (terminate) {
					break;
				}

				jobs_server_cmds_->call_function(msg.at(0), msg);
			}
		}

		logger_->emerg() << "Terminating to receive messages.";
	}
};

#endif // CODEX_WORKER_BROKER_CONNECTION_H

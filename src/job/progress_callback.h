#ifndef RECODEX_WORKER_PROGRESS_CALLBACK_H
#define RECODEX_WORKER_PROGRESS_CALLBACK_H

#include <zmq.hpp>
#include <vector>
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

#include "progress_callback_interface.h"

/**
 * Progress callback implementation which is connected to ZMQ inproc socket.
 * Through this socket information about progress are sent to broker_connection and possibly further.
 * @note No throw implementation... All public methods should be safe to use.
 */
class progress_callback : public progress_callback_interface
{
private:
	/** Socket used to send progress information to broker_connection */
	zmq::socket_t socket_;
	/** String which is used as command in message */
	std::string command_;
	/** False if socket is not connected to another side */
	bool connected_;
	/** Spdlog logger shared among whole project */
	std::shared_ptr<spdlog::logger> logger_;

	/**
	 * If not connected to inproc socket then connect to it.
	 * @throws zmq::error_t if connection failed
	 */
	void connect();

public:
	/**
	 * Construct progress_callback and fill it with given data.
	 * @param context zmq context structure
	 * @param logger pointer to logging class
	 */
	progress_callback(std::shared_ptr<zmq::context_t> context, std::shared_ptr<spdlog::logger> logger);

	virtual void submission_downloaded(const std::string &job_id);

	virtual void job_results_uploaded(const std::string &job_id);

	virtual void job_started(const std::string &job_id);

	virtual void job_ended(const std::string &job_id);

	virtual void task_completed(const std::string &job_id, const std::string &task_id);

	virtual void task_failed(const std::string &job_id, const std::string &task_id);

	virtual void task_skipped(const std::string &job_id, const std::string &task_id);
};

#endif // RECODEX_WORKER_PROGRESS_CALLBACK_H
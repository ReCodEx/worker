#ifndef CODEX_WORKER_PROGRESS_CALLBACK_H
#define CODEX_WORKER_PROGRESS_CALLBACK_H


#include <zmq.hpp>
#include <vector>
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

#include "progress_callback_base.h"


/**
 * Progress callback implementation which is connected to ZMQ inproc socket.
 * Through this socket information about progress are sent to broker_connection and possibly further.
 * @note No throw implementation... All public methods should be safe to use.
 */
class progress_callback : public progress_callback_base
{
private:
	/** Socket which serves to sending progress to broker_connection */
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
	 * @param context zmq contextu structure
	 * @param logger pointer to logging class
	 */
	progress_callback(std::shared_ptr<zmq::context_t> context, std::shared_ptr<spdlog::logger> logger);

	/**
	 * Stated for completion.
	 */
	virtual ~progress_callback();

	/**
	 * Indicates that submission was successfully downloaded from fileserver.
	 * Sends message through zmqsocket to broker_connection.
	 * @param job_id unique identification of downloaded job
	 * @note No throw implementation.
	 */
	virtual void submission_downloaded(const std::string &job_id);
	/**
	 * After calling this, results should be visible for end users.
	 * Sends message through zmqsocket to broker_connection.
	 * @param job_id unique identification of job which results were uploaded
	 * @note No throw implementation.
	 */
	virtual void job_results_uploaded(const std::string &job_id);
	/**
	 * Indicates job was started and all execution machinery was setup and is ready to roll.
	 * Sends message through zmqsocket to broker_connection.
	 * @param job_id unique identification of soon to be evaluated job
	 * @note No throw implementation.
	 */
	virtual void job_started(const std::string &job_id);
	/**
	 * Calling this function should indicate that all was evaluated, just results have to be bubble through.
	 * Sends message through zmqsocket to broker_connection.
	 * @param job_id unique identifier of executed job
	 * @note No throw implementation.
	 */
	virtual void job_ended(const std::string &job_id);
	/**
	 * Tells that task with given particular ID was just successfully completed.
	 * Sends message through zmqsocket to broker_connection.
	 * @param job_id unique identification of job
	 * @param task_id unique identification of successfully completed task
	 * @note No throw implementation.
	 */
	virtual void task_completed(const std::string &job_id, const std::string &task_id);
	/**
	 * Indicates that task with given ID failed in execution.
	 * Information whether task was esential (fatal-failure) is not given.
	 * This should be detected through job_ended callback.
	 * Sends message through zmqsocket to broker_connection.
	 * @param job_id unique identification of job
	 * @param task_id unique identification of failed task
	 * @note No throw implementation.
	 */
	virtual void task_failed(const std::string &job_id, const std::string &task_id);
};


#endif // CODEX_WORKER_PROGRESS_CALLBACK_H

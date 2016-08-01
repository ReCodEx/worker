/*
 * Header file which serves to only purpose,
 *   it's a junkyard of mocked classes which can be used in test cases.
 */

#ifndef RECODEX_WORKER_TESTS_MOCKS_H
#define RECODEX_WORKER_TESTS_MOCKS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/config/worker_config.h"
#include "../src/broker_connection.h"
#include "../src/fileman/file_manager_interface.h"
#include "../src/tasks/task_factory_interface.h"
#include "../src/job/progress_callback_interface.h"
#include "../src/job/job_evaluator_interface.h"

using namespace testing;


/**
 * A mock configuration object. Inverval of pinging is 1 second.
 */
class mock_worker_config : public worker_config
{
public:
	mock_worker_config()
	{
		ON_CALL(*this, get_broker_ping_interval()).WillByDefault(Return(std::chrono::milliseconds(1000)));
	}

	MOCK_CONST_METHOD0(get_broker_uri, std::string());
	MOCK_CONST_METHOD0(get_headers, const worker_config::header_map_t &());
	MOCK_CONST_METHOD0(get_broker_ping_interval, std::chrono::milliseconds());
	MOCK_CONST_METHOD0(get_hwgroup, const std::string &());
	MOCK_CONST_METHOD0(get_worker_id, size_t());
	MOCK_CONST_METHOD0(get_limits, const sandbox_limits &());
};

/**
 * A mock ZeroMQ proxy connection
 */
class mock_connection_proxy
{
public:
	MOCK_METHOD1(connect, void(const std::string &addr));
	MOCK_METHOD1(reconnect_broker, void(const std::string &addr));
	MOCK_METHOD4(poll, void(message_origin::set &, std::chrono::milliseconds, bool &, std::chrono::milliseconds &));
	MOCK_METHOD1(send_broker, bool(const std::vector<std::string> &));
	MOCK_METHOD2(recv_broker, bool(std::vector<std::string> &, bool *));
	MOCK_METHOD1(send_jobs, bool(const std::vector<std::string> &));
	MOCK_METHOD2(recv_jobs, bool(std::vector<std::string> &, bool *));
	MOCK_METHOD2(recv_progress, bool(std::vector<std::string> &, bool *));
};

/**
 * A mock of file manager class.
 */
class mock_file_manager : public file_manager_interface
{
public:
	mock_file_manager()
	{
	}
	MOCK_CONST_METHOD0(get_caching_dir, std::string());
	MOCK_METHOD2(put_file, void(const std::string &name, const std::string &dst_path));
	MOCK_METHOD2(get_file, void(const std::string &src_name, const std::string &dst_path));
};

/**
 * A mock of task factory creator class.
 */
class mock_task_factory : public task_factory_interface
{
public:
	mock_task_factory()
	{
	}
	virtual ~mock_task_factory()
	{
	}
	MOCK_METHOD2(create_internal_task, std::shared_ptr<task_base>(size_t, std::shared_ptr<task_metadata>));
	MOCK_METHOD1(create_sandboxed_task, std::shared_ptr<task_base>(const create_params &));
};

/**
 * A mock of task base class with its own constructors for ids and metadatas.
 */
class mock_task : public task_base
{
public:
	mock_task(size_t id, std::string str_id = "") : task_base(id, std::make_shared<task_metadata>())
	{
		this->task_meta_->task_id = str_id;
	}
	mock_task(size_t id, std::shared_ptr<task_metadata> meta) : task_base(id, meta)
	{
	}
	mock_task() : mock_task(0)
	{
	}
	virtual ~mock_task()
	{
	}

	MOCK_METHOD0(run, std::shared_ptr<task_results>());
};

/**
 * A mock of progress callback class.
 */
class mock_progress_callback : public progress_callback_interface
{
public:
	mock_progress_callback()
	{
	}
	virtual ~mock_progress_callback()
	{
	}

	MOCK_METHOD1(submission_downloaded, void(const std::string &));
	MOCK_METHOD1(submission_failed, void(const std::string &));
	MOCK_METHOD1(job_results_uploaded, void(const std::string &));
	MOCK_METHOD1(job_started, void(const std::string &));
	MOCK_METHOD1(job_ended, void(const std::string &));
	MOCK_METHOD2(task_completed, void(const std::string &, const std::string &));
	MOCK_METHOD2(task_failed, void(const std::string &, const std::string &));
	MOCK_METHOD2(task_skipped, void(const std::string &, const std::string &));
};

/**
 * A mock of job evaluator service.
 */
class mock_job_evaluator : public job_evaluator_interface
{
public:
	mock_job_evaluator()
	{
	}
	MOCK_METHOD1(evaluate, eval_response(eval_request));
};

#endif // RECODEX_WORKER_TESTS_MOCKS_H

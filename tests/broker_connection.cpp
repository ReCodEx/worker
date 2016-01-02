#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/config/worker_config.h"
#include "../src/broker_connection.h"

using namespace testing;

/**
 * A mock configuration object
 */
class mock_worker_config : public worker_config {
public:
	MOCK_CONST_METHOD0(get_broker_uri, std::string());
	MOCK_CONST_METHOD0(get_headers, const worker_config::header_map_t &());
};

/**
 * A mock ZeroMQ proxy connection
 */
class mock_connection_proxy {
public:
	MOCK_METHOD1(connect, void(const std::string &addr));
	MOCK_METHOD3(send, size_t(const char *data, size_t size, int flags));
	MOCK_METHOD1(recv, bool(zmq::message_t &msg));
};

/**
 * A job callback that stores received calls in a vector
 */
struct job_callback {
	std::vector<std::string> calls;

	void operator() (const std::string &job_id, const std::string &job_url, const std::string &result_url)
	{
		calls.push_back("call");
	}
};

TEST(broker_connection, sends_init)
{
	mock_worker_config config;
	StrictMock<mock_connection_proxy> proxy;
	job_callback callback;
	broker_connection<mock_connection_proxy, job_callback> connection(config, &proxy, &callback);

	std::string addr("tcp://localhost:9876");
	worker_config::header_map_t headers = {
		std::make_pair("env", "c"),
		std::make_pair("hwgroup", "group_1")
	};

	EXPECT_CALL(config, get_broker_uri())
		.Times(2)
		.WillRepeatedly(Return(addr));

	EXPECT_CALL(config, get_headers())
		.Times(1)
		.WillOnce(ReturnRef(headers));

	{
		InSequence s;

		EXPECT_CALL(proxy, connect(StrEq(addr)))
			.Times(1);

		EXPECT_CALL(proxy, send(StartsWith("init"), 4, ZMQ_SNDMORE))
			.Times(1);

		EXPECT_CALL(proxy, send(StartsWith("env=c"), 5, ZMQ_SNDMORE))
			.Times(1);

		EXPECT_CALL(proxy, send(StartsWith("hwgroup=group_1"), 15, 0))
			.Times(1);
	}

	connection.connect();
}

/**
 * Copies the message pointed to by @a msg to the first argument of the mock function
 */
ACTION_P(SetMsg, msg)
{
	((zmq::message_t &) arg0).copy(msg);
}

TEST(broker_connection, calls_callback)
{
	mock_worker_config config;
	StrictMock<mock_connection_proxy> proxy;
	job_callback callback;
	broker_connection<mock_connection_proxy, job_callback> connection(config, &proxy, &callback);

	zmq::message_t msg(5);

	{
		InSequence s;
		EXPECT_CALL(proxy, recv(_))
			.Times(1)
			.WillOnce(DoAll(
				SetMsg(&msg),
				Return(true)
			));

		EXPECT_CALL(proxy, recv(_))
			.Times(1)
			.WillOnce(Throw(zmq::error_t()));
	}

	connection.receive_tasks();
	ASSERT_EQ(callback.calls.size(), 1);
}

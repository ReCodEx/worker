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
	MOCK_METHOD1(send, bool(const std::vector<std::string> &));
	MOCK_METHOD2(recv, bool(std::vector<std::string> &, bool *));
};

/**
 * A job callback that stores received calls in a vector
 */
struct test_callback {
	std::vector<job_request> calls;

	void operator() (job_request request)
	{
		calls.push_back(request);
	}
};

TEST(broker_connection, sends_init)
{
	mock_worker_config config;
	auto proxy = std::make_shared<StrictMock<mock_connection_proxy>>();
	test_callback callback;
	broker_connection<mock_connection_proxy, test_callback> connection(config, proxy, &callback);

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

		EXPECT_CALL(*proxy, connect(StrEq(addr)));
		EXPECT_CALL(*proxy, send(ElementsAre(
			"init",
			"env=c",
			"hwgroup=group_1"
		))).WillOnce(Return(true));
	}

	connection.connect();
}

TEST(broker_connection, calls_callback)
{
	mock_worker_config config;
	auto proxy = std::make_shared<StrictMock<mock_connection_proxy>>();
	test_callback callback;
	broker_connection<mock_connection_proxy, test_callback> connection(config, proxy, &callback);

	{
		InSequence s;

		EXPECT_CALL(*proxy, recv(_, _))
			.Times(1)
			.WillOnce(DoAll(
				SetArgReferee<0>(std::vector<std::string>{
					"eval",
					"10",
					"http://localhost:5487/submission_archives/10.tar.gz",
					"http://localhost:5487/results/10",
				}),
				Return(true)
			));

		EXPECT_CALL(*proxy, recv(_, _))
			.WillRepeatedly(DoAll(
				SetArgPointee<1>(true),
				Return(false)
			));
	}

	connection.receive_tasks();
	ASSERT_EQ(callback.calls.size(), 1);

	job_request request = callback.calls.at(0);
	ASSERT_EQ(request.job_id, "10");
	ASSERT_EQ(request.job_url, "http://localhost:5487/submission_archives/10.tar.gz");
	ASSERT_EQ(request.result_url, "http://localhost:5487/results/10");
}

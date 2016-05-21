#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../src/broker_connection.h"
#include "../src/config/worker_config.h"

using namespace testing;

/**
 * A mock configuration object
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
};

TEST(broker_connection, sends_init)
{
	auto config = std::make_shared<mock_worker_config>();
	auto proxy = std::make_shared<StrictMock<mock_connection_proxy>>();
	broker_connection<mock_connection_proxy> connection(config, proxy);

	std::string addr("tcp://localhost:9876");
	worker_config::header_map_t headers = {std::make_pair("env", "c"), std::make_pair("threads", "2")};

	EXPECT_CALL(*config, get_broker_uri()).WillRepeatedly(Return(addr));

	EXPECT_CALL(*config, get_headers()).WillRepeatedly(ReturnRef(headers));

	std::string hwgroup = "group_1";
	EXPECT_CALL(*config, get_hwgroup()).WillRepeatedly(ReturnRef(hwgroup));

	{
		InSequence s;

		EXPECT_CALL(*proxy, connect(StrEq(addr)));
		EXPECT_CALL(*proxy, send_broker(ElementsAre("init", hwgroup, "env=c", "threads=2"))).WillOnce(Return(true));
	}

	connection.connect();
}

ACTION(ClearFlags)
{
	((message_origin::set &) arg0).reset();
}

ACTION_P(SetFlag, flag)
{
	((message_origin::set &) arg0).set(flag, true);
}

TEST(broker_connection, forwards_eval)
{
	auto config = std::make_shared<mock_worker_config>();
	auto proxy = std::make_shared<StrictMock<mock_connection_proxy>>();
	broker_connection<mock_connection_proxy> connection(config, proxy);

	EXPECT_CALL(*proxy, send_broker(ElementsAre("ping"))).WillRepeatedly(Return(true));

	{
		InSequence s;

		EXPECT_CALL(*proxy, poll(_, _, _, _)).WillOnce(DoAll(ClearFlags(), SetFlag(message_origin::BROKER)));

		EXPECT_CALL(*proxy, recv_broker(_, _))
			.Times(1)
			.WillOnce(DoAll(SetArgReferee<0>(std::vector<std::string>{
								"eval",
								"10",
								"http://localhost:5487/submission_archives/10.tar.gz",
								"http://localhost:5487/results/10",
							}),
				Return(true)));

		EXPECT_CALL(*proxy,
			send_jobs(ElementsAre("eval",
				"10",
				"http://localhost:5487/submission_archives/10.tar.gz",
				"http://localhost:5487/results/10")))
			.WillOnce(Return(true));

		EXPECT_CALL(*proxy, poll(_, _, _, _)).WillRepeatedly(SetArgReferee<2>(true));
	}

	connection.receive_tasks();
}

TEST(broker_connection, sends_ping)
{
	auto config = std::make_shared<NiceMock<mock_worker_config>>();
	auto proxy = std::make_shared<StrictMock<mock_connection_proxy>>();
	broker_connection<mock_connection_proxy> connection(config, proxy);

	EXPECT_CALL(*config, get_broker_ping_interval()).WillRepeatedly(Return(std::chrono::milliseconds(1100)));

	EXPECT_CALL(*proxy, send_broker(ElementsAre("ping"))).Times(AtLeast(1));

	{
		InSequence s;

		EXPECT_CALL(*proxy, poll(_, Le(std::chrono::milliseconds(1100)), _, _))
			.WillOnce(DoAll(ClearFlags(), SetArgReferee<3>(std::chrono::milliseconds(600))));

		EXPECT_CALL(*proxy, poll(_, Le(std::chrono::milliseconds(500)), _, _))
			.WillOnce(DoAll(ClearFlags(), SetArgReferee<3>(std::chrono::milliseconds(600))));

		EXPECT_CALL(*proxy, poll(_, Le(std::chrono::milliseconds(1100)), _, _)).WillRepeatedly(SetArgReferee<2>(true));
	}

	connection.receive_tasks();
}

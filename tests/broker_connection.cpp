#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/worker_config.hpp"
#include "../src/broker_connection.hpp"

using testing::Return;

class worker_config_mock : public worker_config {
public:
	MOCK_METHOD0(get_broker_uri, std::string());
};

struct task_callback {
	void operator() ()
	{
	}
};

TEST(broker_connection, sends_init)
{
	worker_config_mock config;

	EXPECT_CALL(config, get_broker_uri())
			.Times(1)
			.WillRepeatedly(Return("tcp://localhost:9876"));

	broker_connection connection(config);
	connection.receive_tasks<task_callback>();

}

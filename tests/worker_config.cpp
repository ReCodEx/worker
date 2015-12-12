#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/worker_config.hpp"

TEST(worker_config, load_yaml_basic)
{
	auto yaml = YAML::Load(
		"broker: tcp://localhost:1234\n"
		"headers:\n"
		"    env:\n"
		"        - c\n"
		"        - python\n"
		"    threads: 10\n"
		"    hwgroup: group_1\n"
	);

	worker_config config(yaml);

	worker_config::header_map_t expected_headers = {
		{"env", "c"},
		{"env", "python"},
		{"threads", "10"},
		{"hwgroup", "group_1"}
	};

	ASSERT_STREQ("tcp://localhost:1234", config.get_broker_uri().c_str());
	ASSERT_EQ(expected_headers, config.get_headers());
}

/**
 * Map as a header value causes an exception
 */
TEST(worker_config, invalid_header_value_1)
{
	auto yaml = YAML::Load(
		"broker: tcp://localhost:1234\n"
		"headers:\n"
		"    env:\n"
		"        foo: c\n"
		"    threads: 10\n"
		"    hwgroup: group_1\n"
	);

	ASSERT_THROW(worker_config config(yaml), config_error);
}

/**
 * Map in a header value sequence causes an exception
 */
TEST(worker_config, invalid_header_value_2)
{
	auto yaml = YAML::Load(
		"broker: tcp://localhost:1234\n"
		"headers:\n"
		"    env:\n"
		"        - foo: c\n"
		"    threads: 10\n"
		"    hwgroup: group_1\n"
	);

	ASSERT_THROW(worker_config config(yaml), config_error);
}

/**
 * Non-scalar broker URI causes an exception
 */
TEST(worker_config, invalid_broker_uri)
{
	auto yaml = YAML::Load(
		"broker:\n"
		"    tcp: localhost:1234\n"
		"headers:\n"
		"    env:\n"
		"        - foo: c\n"
		"    threads: 10\n"
		"    hwgroup: group_1\n"
	);

	ASSERT_THROW(worker_config config(yaml), config_error);
}

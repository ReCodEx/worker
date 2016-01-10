#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/config/worker_config.h"

TEST(worker_config, load_yaml_basic)
{
	auto yaml = YAML::Load(
		"---\n"
		"worker-id: eval_1\n"
		"broker-uri: tcp://localhost:1234\n"
		"headers:\n"
		"    env:\n"
		"        - c\n"
		"        - python\n"
		"    threads: 10\n"
		"    hwgroup: group_1\n"
		"file-manager:\n"
		"    file-collector:\n"
		"        hostname: localhost\n"
		"        port: 80\n"
		"        username: 654321\n"
		"        password: 123456\n"
		"    cache:\n"
		"        cache-dir: /tmp/cache\n"
		"logger:\n"
		"    file: /var/log/isoeval\n"
		"    level: emerg\n"
		"    max-size: 2048576\n"
		"    rotations: 5\n"
		"limits:\n"
		"    time: 5\n"
		"    wall-time: 6\n"
		"    extra-time: 2\n"
		"    stack-size: 50000\n"
		"    memory: 60000\n"
		"    parallel: false\n"
		"    disk-blocks: 50\n"
		"    disk-inodes: 7\n"
		"sandboxes-wrap-limits:\n"
		"    - name: isolate\n"
		"      time: 10\n"
		"    - name: csharp\n"
		"      time: 20\n"
		"bound-directories:\n"
		"    localbin: /usr/local/bin\n"
		"    share: /usr/share\n"
		"..."
	);

	worker_config config(yaml);

	worker_config::header_map_t expected_headers = {
		{"env", "c"},
		{"env", "python"},
		{"threads", "10"},
		{"hwgroup", "group_1"}
	};

	std::map<std::string, size_t> expected_sand_limits = {
		{ "isolate", 10 },
		{ "csharp", 20 }
	};

	sandbox_limits expected_limits;
	expected_limits.memory_usage = 60000;
	expected_limits.cpu_time = 5;
	expected_limits.wall_time = 6;
	expected_limits.extra_time = 2;
	expected_limits.stack_size = 50000;
	expected_limits.disk_blocks = 50;
	expected_limits.disk_inodes = 7;
	expected_limits.bound_dirs = {
		std::make_pair("localbin", "/usr/local/bin"),
		std::make_pair("share", "/usr/share")
	};

	log_config expected_log;
	expected_log.log_path = "/var/log";
	expected_log.log_basename = "isoeval";
	expected_log.log_level = "emerg";
	expected_log.log_file_size = 2048576;
	expected_log.log_files_count = 5;

	fileman_config expected_fileman;
	expected_fileman.hostname = "localhost";
	expected_fileman.port = 80;
	expected_fileman.username = "654321";
	expected_fileman.password = "123456";
	expected_fileman.cache_dir = "/tmp/cache";

	ASSERT_STREQ("tcp://localhost:1234", config.get_broker_uri().c_str());
	ASSERT_STREQ("eval_1", config.get_worker_id().c_str());
	ASSERT_EQ(expected_headers, config.get_headers());
	ASSERT_EQ(expected_sand_limits, config.get_sandboxes_limits());
	ASSERT_EQ(expected_limits, config.get_limits());
	ASSERT_EQ(expected_log, config.get_log_config());
	ASSERT_EQ(expected_fileman, config.get_fileman_config());
}

/**
 * Map as a header value causes an exception
 */
TEST(worker_config, invalid_header_value_1)
{
	auto yaml = YAML::Load(
		"worker-id: 1\n"
		"broker-uri: tcp://localhost:1234\n"
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
		"worker-id: 1\n"
		"broker-uri: tcp://localhost:1234\n"
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
		"worker-id: 1\n"
		"broker-uri:\n"
		"    tcp: localhost:1234\n"
		"headers:\n"
		"    env:\n"
		"        - foo: c\n"
		"    threads: 10\n"
		"    hwgroup: group_1\n"
	);

	ASSERT_THROW(worker_config config(yaml), config_error);
}

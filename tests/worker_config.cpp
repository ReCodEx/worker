#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/config/worker_config.h"

TEST(worker_config, load_yaml_basic)
{
	auto yaml = YAML::Load(
		"---\n"
		"worker-id: 8\n"
		"broker-uri: tcp://localhost:1234\n"
		"headers:\n"
		"    env:\n"
		"        - c\n"
		"        - python\n"
		"    threads: 10\n"
		"    hwgroup: group_1\n"
		"file-managers:\n"
		"    - hostname: http://localhost:80\n"
		"      username: 654321\n"
		"      password: 123456\n"
		"    - hostname: http://localhost:4242\n"
		"      username: 123456\n"
		"      password: 654321\n"
		"file-cache:\n"
		"    cache-dir: /tmp/isoeval/cache\n"
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
		"    parallel: 1\n"
		"    disk-size: 50\n"
		"    disk-files: 7\n"
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
	expected_limits.processes = 1;
	expected_limits.stack_size = 50000;
	expected_limits.disk_size = 50;
	expected_limits.disk_files = 7;
	expected_limits.bound_dirs = {
		{ "localbin", "/usr/local/bin" },
		{ "share", "/usr/share" }
	};

	log_config expected_log;
	expected_log.log_path = "/var/log";
	expected_log.log_basename = "isoeval";
	expected_log.log_level = "emerg";
	expected_log.log_file_size = 2048576;
	expected_log.log_files_count = 5;

	std::vector<fileman_config> expected_filemans;
	fileman_config expected_fileman;
	expected_fileman.remote_url = "http://localhost:80";
	expected_fileman.username = "654321";
	expected_fileman.password = "123456";
	expected_filemans.push_back(expected_fileman);
	expected_fileman.remote_url = "http://localhost:4242";
	expected_fileman.username = "123456";
	expected_fileman.password = "654321";
	expected_filemans.push_back(expected_fileman);

	ASSERT_STREQ("tcp://localhost:1234", config.get_broker_uri().c_str());
	ASSERT_EQ((size_t) 8, config.get_worker_id());
	ASSERT_STREQ("/tmp/isoeval/cache", config.get_cache_dir().c_str());
	ASSERT_EQ(expected_headers, config.get_headers());
	ASSERT_EQ(expected_sand_limits, config.get_sandboxes_limits());
	ASSERT_EQ(expected_limits, config.get_limits());
	ASSERT_EQ(expected_log, config.get_log_config());
	ASSERT_EQ(expected_filemans, config.get_filemans_configs());
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "../src/helpers/config.h"
using namespace helpers;


TEST(job_config_test, bad_format)
{
	auto yaml = YAML::Load("");

	// empty configuration
	EXPECT_THROW(build_job_metadata(yaml), config_exception);

	// tasks is not a sequence
	yaml = YAML::Load(
		"---\n"
		"submission:\n"
		"    job-id: 5\n"
		"    language: cpp\n"
		"    file-collector: localhost\n"
		"tasks: hello\n"
		"...\n"
	);
	EXPECT_THROW(build_job_metadata(yaml), config_exception);

	// submission is not a map
	yaml = YAML::Load(
		"---\n"
		"submission: hello\n"
		"tasks:\n"
		"    - task-id: cp\n"
		"      priority: 1\n"
		"      fatal-failure: true\n"
		"      cmd:\n"
		"          bin: cp\n"
		"          args:\n"
		"              - hello.cpp\n"
		"              - hello_world.cpp\n"
		"...\n"
	);
	EXPECT_THROW(build_job_metadata(yaml), config_exception);

	// item job-id	missing in submission
	yaml = YAML::Load(
		"---\n"
		"submission:\n"
		"    language: cpp\n"
		"    file-collector: localhost\n"
		"tasks:\n"
		"    - task-id: cp\n"
		"      priority: 1\n"
		"      fatal-failure: true\n"
		"      cmd:\n"
		"          bin: cp\n"
		"          args:\n"
		"              - hello.cpp\n"
		"              - hello_world.cpp\n"
		"...\n"
	);
	EXPECT_THROW(build_job_metadata(yaml), config_exception);

	// priority missing in internal task
	yaml = YAML::Load(
		"---\n"
		"submission:\n"
		"    job-id: 5\n"
		"    language: cpp\n"
		"    file-collector: localhost\n"
		"tasks:\n"
		"    - task-id: cp\n"
		"      fatal-failure: true\n"
		"      cmd:\n"
		"          bin: cp\n"
		"          args:\n"
		"              - hello.cpp\n"
		"              - hello_world.cpp\n"
		"...\n"
	);
	EXPECT_THROW(build_job_metadata(yaml), config_exception);

#ifndef _WIN32
	// task-id missing in external task
	yaml = YAML::Load(
		"---\n"
		"submission:\n"
		"    job-id: 5\n"
		"    language: cpp\n"
		"    file-collector: localhost\n"
		"tasks:\n"
		"    - task-id: cp\n"
		"      priority: 1\n"
		"      fatal-failure: true\n"
		"      cmd:\n"
		"          bin: cp\n"
		"          args:\n"
		"              - hello.cpp\n"
		"              - hello_world.cpp\n"
		"    - priority: 4\n"
		"      fatal-failure: false\n"
		"      dependencies:\n"
		"          - cp\n"
		"      cmd:\n"
		"          bin: recodex\n"
		"          args:\n"
		"              - -v\n"
		"              - \"-f 01.in\"\n"
		"      stdin: 01.in\n"
		"      stdout: 01.out\n"
		"      stderr: 01.err\n"
		"      sandbox:\n"
		"          name: isolate\n"
		"          limits:\n"
		"              - hw-group-id: group1\n"
		"                time: 5\n"
		"                wall-time: 5\n"
		"                extra-time: 5\n"
		"                stack-size: 50000\n"
		"                memory: 50000\n"
		"                parallel: 1\n"
		"                disk-blocks: 50\n"
		"                disk-inodes: 5\n"
		"                environ-variable:\n"
		"                    ISOLATE_BOX: /box\n"
		"                    ISOLATE_TMP: /tmp\n"
		"...\n"
	);
	EXPECT_THROW(build_job_metadata(yaml), config_exception);
#endif
}

TEST(job_config_test, correct_format)
{
#ifndef _WIN32
	// correct configuration
	auto yaml = YAML::Load(
		"---\n"
		"submission:\n"
		"    job-id: 5\n"
		"    language: cpp\n"
		"    file-collector: localhost\n"
		"tasks:\n"
		"    - task-id: cp\n"
		"      priority: 1\n"
		"      fatal-failure: true\n"
		"      cmd:\n"
		"          bin: cp\n"
		"          args:\n"
		"              - hello.cpp\n"
		"              - hello_world.cpp\n"
		"    - task-id: eval\n"
		"      priority: 4\n"
		"      fatal-failure: false\n"
		"      dependencies:\n"
		"          - cp\n"
		"      cmd:\n"
		"          bin: recodex\n"
		"          args:\n"
		"              - -v\n"
		"              - \"-f 01.in\"\n"
		"      stdin: 01.in\n"
		"      stdout: 01.out\n"
		"      stderr: 01.err\n"
		"      sandbox:\n"
		"          name: isolate\n"
		"          limits:\n"
		"              - hw-group-id: group1\n"
		"                time: 5\n"
		"                wall-time: 5\n"
		"                extra-time: 5\n"
		"                stack-size: 50000\n"
		"                memory: 50000\n"
		"                parallel: 1\n"
		"                disk-blocks: 50\n"
		"                disk-inodes: 5\n"
		"                environ-variable:\n"
		"                    ISOLATE_BOX: /box\n"
		"                    ISOLATE_TMP: /tmp\n"
		"                bound-directories:\n"
		"                    /tmp: /recodex/tmp\n"
		"...\n"
	);
	build_job_metadata(yaml);
	EXPECT_NO_THROW(build_job_metadata(yaml));
#endif
}

TEST(job_config_test, config_data)
{
	auto yaml = YAML::Load(
		"---\n"
		"submission:\n"
		"    job-id: 5\n"
		"    language: cpp\n"
		"    file-collector: localhost\n"
		"tasks:\n"
		"    - task-id: cp\n"
		"      priority: 1\n"
		"      fatal-failure: true\n"
		"      cmd:\n"
		"          bin: cp\n"
		"          args:\n"
		"              - hello.cpp\n"
		"              - hello_world.cpp\n"
		"    - task-id: eval\n"
		"      priority: 4\n"
		"      fatal-failure: false\n"
		"      dependencies:\n"
		"          - cp\n"
		"      cmd:\n"
		"          bin: recodex\n"
		"          args:\n"
		"              - -v\n"
		"              - \"-f 01.in\"\n"
		"      stdin: 01.in\n"
		"      stdout: 01.out\n"
		"      stderr: 01.err\n"
		"      sandbox:\n"
		"          name: isolate\n"
		"          limits:\n"
		"              - hw-group-id: group1\n"
		"                time: 5\n"
		"                wall-time: 6\n"
		"                extra-time: 7\n"
		"                stack-size: 50000\n"
		"                memory: 60000\n"
		"                parallel: 1\n"
		"                disk-blocks: 50\n"
		"                disk-inodes: 10\n"
		"                chdir: /eval\n"
		"                environ-variable:\n"
		"                    ISOLATE_BOX: /box\n"
		"                    ISOLATE_TMP: /tmp\n"
		"                bound-directories:\n"
		"                    /tmp/recodex: /recodex/tmp\n"
		"              - hw-group-id: group2\n"
		"...\n"
	);

	// empty configuration
	auto result = build_job_metadata(yaml);

	ASSERT_EQ(result->job_id, "5");
	ASSERT_EQ(result->language, "cpp");
	ASSERT_EQ(result->file_server_url, "localhost");

	auto task1 = result->tasks[0];
	ASSERT_EQ(task1->task_id, "cp");
	ASSERT_EQ(task1->priority, 1);
	ASSERT_EQ(task1->fatal_failure, true);
	ASSERT_EQ(task1->dependencies.size(), 0);
	ASSERT_EQ(task1->binary, "cp");
	ASSERT_EQ(task1->cmd_args[0], "hello.cpp");
	ASSERT_EQ(task1->cmd_args[1], "hello_world.cpp");
	ASSERT_EQ(task1->std_input, "");
	ASSERT_EQ(task1->std_output, "");
	ASSERT_EQ(task1->std_error, "");
	ASSERT_EQ(task1->sandbox, nullptr);

	auto task2 = result->tasks[1];
	ASSERT_EQ(task2->task_id, "eval");
	ASSERT_EQ(task2->priority, 4);
	ASSERT_EQ(task2->fatal_failure, false);
	ASSERT_EQ(task2->dependencies.size(), 1);
	ASSERT_EQ(task2->dependencies[0], "cp");
	ASSERT_EQ(task2->binary, "recodex");
	ASSERT_EQ(task2->cmd_args[0], "-v");
	ASSERT_EQ(task2->cmd_args[1], "-f 01.in");
	ASSERT_EQ(task2->std_input, "01.in");
	ASSERT_EQ(task2->std_output, "01.out");
	ASSERT_EQ(task2->std_error, "01.err");
	ASSERT_NE(task2->sandbox, nullptr);
	ASSERT_EQ(task2->sandbox->name, "isolate");
	ASSERT_EQ(task2->sandbox->limits.size(), 2);
	EXPECT_NO_THROW(task2->sandbox->limits.at("group1"));
	EXPECT_NO_THROW(task2->sandbox->limits.at("group2"));

	auto limit1 = task2->sandbox->limits.at("group1");
	ASSERT_EQ(limit1->cpu_time, 5);
	ASSERT_EQ(limit1->wall_time, 6);
	ASSERT_EQ(limit1->extra_time, 7);
	ASSERT_EQ(limit1->memory_usage, 60000);
	ASSERT_EQ(limit1->stack_size, 50000);
	ASSERT_EQ(limit1->processes, 1);
	ASSERT_EQ(limit1->chdir, "/eval");
	ASSERT_EQ(limit1->disk_blocks, 50);
	ASSERT_EQ(limit1->disk_inodes, 10);
	std::map<std::string, std::string> expected_environ = {
		{ "ISOLATE_BOX", "/box" },
		{ "ISOLATE_TMP", "/tmp" }
	};
	ASSERT_EQ(limit1->environ_vars, expected_environ);
	std::map<std::string, std::string> expected_bound_dirs = {
		{ "/tmp/recodex", "/recodex/tmp" }
	};
	ASSERT_EQ(limit1->bound_dirs, expected_bound_dirs);

	auto limit2 = task2->sandbox->limits.at("group2");
	ASSERT_EQ(limit2->cpu_time, FLT_MAX);
	ASSERT_EQ(limit2->wall_time, FLT_MAX);
	ASSERT_EQ(limit2->extra_time, FLT_MAX);
	ASSERT_EQ(limit2->memory_usage, SIZE_MAX);
	ASSERT_EQ(limit2->stack_size, SIZE_MAX);
	ASSERT_EQ(limit2->processes, 0);
	ASSERT_EQ(limit2->chdir, "");
	ASSERT_EQ(limit2->disk_blocks, SIZE_MAX);
	ASSERT_EQ(limit2->disk_inodes, SIZE_MAX);
	ASSERT_EQ(limit2->environ_vars.size(), 0);
	ASSERT_EQ(limit2->bound_dirs.size(), 0);
}

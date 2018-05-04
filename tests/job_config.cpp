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
	yaml = YAML::Load("---\n"
					  "submission:\n"
					  "    job-id: 5\n"
					  "    hw-groups:\n"
					  "        - group1\n"
					  "    file-collector: localhost\n"
					  "tasks: hello\n"
					  "...\n");
	EXPECT_THROW(build_job_metadata(yaml), config_exception);

	// submission is not a map
	yaml = YAML::Load("---\n"
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
					  "...\n");
	EXPECT_THROW(build_job_metadata(yaml), config_exception);

	// item job-id	missing in submission
	yaml = YAML::Load("---\n"
					  "submission:\n"
					  "    file-collector: localhost\n"
					  "    hw-groups:\n"
					  "        - group1\n"
					  "tasks:\n"
					  "    - task-id: cp\n"
					  "      priority: 1\n"
					  "      fatal-failure: true\n"
					  "      cmd:\n"
					  "          bin: cp\n"
					  "          args:\n"
					  "              - hello.cpp\n"
					  "              - hello_world.cpp\n"
					  "...\n");
	EXPECT_THROW(build_job_metadata(yaml), config_exception);

	// task-id missing in external task
	yaml = YAML::Load("---\n"
					  "submission:\n"
					  "    job-id: 5\n"
					  "    file-collector: localhost\n"
					  "    hw-groups:\n"
					  "        - group1\n"
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
					  "      sandbox:\n"
					  "          name: fake\n"
					  "          stdin: 01.in\n"
					  "          stdout: 01.out\n"
					  "          stderr: 01.err\n"
					  "          limits:\n"
					  "              - hw-group-id: group1\n"
					  "                time: 5\n"
					  "                wall-time: 5\n"
					  "                extra-time: 5\n"
					  "                stack-size: 50000\n"
					  "                memory: 50000\n"
					  "                parallel: 1\n"
					  "                disk-size: 50\n"
					  "                disk-files: 5\n"
					  "                environ-variable:\n"
					  "                    ISOLATE_BOX: /box\n"
					  "                    ISOLATE_TMP: /tmp\n"
					  "...\n");
	EXPECT_THROW(build_job_metadata(yaml), config_exception);
}

TEST(job_config_test, correct_format)
{
	// correct configuration
	auto yaml = YAML::Load("---\n"
						   "submission:\n"
						   "    job-id: 5\n"
						   "    file-collector: localhost\n"
						   "    hw-groups:\n"
						   "        - group1\n"
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
						   "      sandbox:\n"
						   "          name: fake\n"
						   "          stdin: 01.in\n"
						   "          stdout: 01.out\n"
						   "          stderr: 01.err\n"
						   "          stderr-to-stdout: false\n"
						   "          working-directory: working\n"
						   "          limits:\n"
						   "              - hw-group-id: group1\n"
						   "                time: 5\n"
						   "                wall-time: 5\n"
						   "                extra-time: 5\n"
						   "                stack-size: 50000\n"
						   "                memory: 50000\n"
						   "                parallel: 1\n"
						   "                disk-size: 50\n"
						   "                disk-files: 5\n"
						   "                environ-variable:\n"
						   "                    ISOLATE_BOX: /box\n"
						   "                    ISOLATE_TMP: /tmp\n"
						   "                bound-directories:\n"
						   "                    - src: /tmp\n"
						   "                      dst: /recodex/tmp\n"
						   "...\n");
	build_job_metadata(yaml);
	EXPECT_NO_THROW(build_job_metadata(yaml));
}

TEST(job_config_test, config_data)
{
	using sp = sandbox_limits::dir_perm;
	auto yaml = YAML::Load("---\n"
						   "submission:\n"
						   "    job-id: 5\n"
						   "    file-collector: localhost\n"
						   "    log: true\n"
						   "    hw-groups:\n"
						   "        - group1\n"
						   "tasks:\n"
						   "    - task-id: cp\n"
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
						   "      sandbox:\n"
						   "          name: fake\n"
						   "          stdin: 01.in\n"
						   "          stdout: 01.out\n"
						   "          stderr: 01.err\n"
						   "          stderr-to-stdout: true\n"
						   "          chdir: /eval\n"
						   "          working-directory: working\n"
						   "          limits:\n"
						   "              - hw-group-id: group1\n"
						   "                time: 5\n"
						   "                wall-time: 6\n"
						   "                extra-time: 7\n"
						   "                stack-size: 50000\n"
						   "                memory: 60000\n"
						   "                parallel: 1\n"
						   "                disk-size: 50\n"
						   "                disk-files: 10\n"
						   "                environ-variable:\n"
						   "                    ISOLATE_BOX: /box\n"
						   "                    ISOLATE_TMP: /tmp\n"
						   "                bound-directories:\n"
						   "                    - src: /tmp/recodex\n"
						   "                      dst: /recodex/tmp\n"
						   "                      mode: RW,NOEXEC\n"
						   "              - hw-group-id: group2\n"
						   "...\n");

	// empty configuration
	auto result = build_job_metadata(yaml);

	ASSERT_EQ(result->job_id, "5");
	ASSERT_EQ(result->file_server_url, "localhost");
	ASSERT_EQ(result->log, true);
	ASSERT_EQ(result->hwgroups.size(), 1u);
	ASSERT_EQ(result->hwgroups.at(0), "group1");

	auto task1 = result->tasks[0];
	ASSERT_EQ(task1->task_id, "cp");
	ASSERT_EQ(task1->priority, 1u); // default value
	ASSERT_EQ(task1->fatal_failure, true);
	ASSERT_EQ(task1->dependencies.size(), 0u);
	ASSERT_EQ(task1->binary, "cp");
	ASSERT_EQ(task1->cmd_args[0], "hello.cpp");
	ASSERT_EQ(task1->cmd_args[1], "hello_world.cpp");
	ASSERT_EQ(task1->sandbox, nullptr);

	auto task2 = result->tasks[1];
	ASSERT_EQ(task2->task_id, "eval");
	ASSERT_EQ(task2->priority, 4u);
	ASSERT_EQ(task2->fatal_failure, false);
	ASSERT_EQ(task2->dependencies.size(), 1u);
	ASSERT_EQ(task2->dependencies[0], "cp");
	ASSERT_EQ(task2->binary, "recodex");
	ASSERT_EQ(task2->cmd_args[0], "-v");
	ASSERT_EQ(task2->cmd_args[1], "-f 01.in");

	ASSERT_NE(task2->sandbox, nullptr);
	ASSERT_EQ(task2->sandbox->name, "fake");
	ASSERT_EQ(task2->sandbox->std_input, "01.in");
	ASSERT_EQ(task2->sandbox->std_output, "01.out");
	ASSERT_EQ(task2->sandbox->std_error, "01.err");
	ASSERT_TRUE(task2->sandbox->stderr_to_stdout);
	ASSERT_EQ(task2->sandbox->chdir, "/eval");
	ASSERT_EQ(task2->sandbox->working_directory, "working");

	ASSERT_EQ(task2->sandbox->loaded_limits.size(), 2u);
	EXPECT_NO_THROW(task2->sandbox->loaded_limits.at("group1"));
	EXPECT_NO_THROW(task2->sandbox->loaded_limits.at("group2"));

	auto limit1 = task2->sandbox->loaded_limits.at("group1");
	ASSERT_EQ(limit1->cpu_time, 5);
	ASSERT_EQ(limit1->wall_time, 6);
	ASSERT_EQ(limit1->extra_time, 7);
	ASSERT_EQ(limit1->memory_usage, 60000u);
	ASSERT_EQ(limit1->stack_size, 50000u);
	ASSERT_EQ(limit1->processes, 1u);
	ASSERT_EQ(limit1->disk_size, 50u);
	ASSERT_EQ(limit1->disk_files, 10u);

	// Both combinations are valid (YAML doesn't sort them)
	std::vector<std::pair<std::string, std::string>> expected_environ_1 = {
		{"ISOLATE_TMP", "/tmp"}, {"ISOLATE_BOX", "/box"}};
	std::vector<std::pair<std::string, std::string>> expected_environ_2 = {
		{"ISOLATE_BOX", "/box"}, {"ISOLATE_TMP", "/tmp"}};
	if (limit1->environ_vars[0].first == "ISOLATE_TMP") {
		ASSERT_EQ(limit1->environ_vars, expected_environ_1);
	} else {
		ASSERT_EQ(limit1->environ_vars, expected_environ_2);
	}

	std::vector<std::tuple<std::string, std::string, sp>> expected_bound_dirs = {
		std::tuple<std::string, std::string, sp>{"/tmp/recodex", "/recodex/tmp", static_cast<sp>(sp::RW | sp::NOEXEC)}};
	ASSERT_EQ(limit1->bound_dirs, expected_bound_dirs);

	auto limit2 = task2->sandbox->loaded_limits.at("group2");
	ASSERT_EQ(limit2->cpu_time, FLT_MAX);
	ASSERT_EQ(limit2->wall_time, FLT_MAX);
	ASSERT_EQ(limit2->extra_time, FLT_MAX);
	ASSERT_EQ(limit2->memory_usage, SIZE_MAX);
	ASSERT_EQ(limit2->stack_size, SIZE_MAX);
	ASSERT_EQ(limit2->processes, SIZE_MAX);
	ASSERT_EQ(limit2->disk_size, SIZE_MAX);
	ASSERT_EQ(limit2->disk_files, SIZE_MAX);
	ASSERT_EQ(limit2->environ_vars.size(), 0u);
	ASSERT_EQ(limit2->bound_dirs.size(), 0u);
}

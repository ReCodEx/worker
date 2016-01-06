#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "../src/tasks/job.h"
#include "../src/fileman/cache_manager.h"


TEST(job_test, bad_parameters)
{
	auto yaml = YAML::Load("");

	// config not given
	EXPECT_THROW(job j(yaml, "", nullptr, nullptr, nullptr), job_exception);

	// file manager not given
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	EXPECT_THROW(job j(yaml, "", nullptr, conf_ptr, nullptr), job_exception);

	// source code path not given
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);
	EXPECT_THROW(job j(yaml, "", nullptr, conf_ptr, fileman_ptr), job_exception);
}

TEST(job_test, bad_paths)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path();
	dir_root = dir_root / "isoeval";
	path dir = dir_root;
	dir = dir / "job_test";
	auto yaml = YAML::Load("");
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);

	// non-existing source code folder
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// source code path with no source codes in it
	create_directories(dir);
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// source code directory is not a directory
	dir = dir / "hello";
	std::ofstream hello(dir.string());
	hello << "hello" << std::endl;
	hello.close();
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, config_format)
{
	// prepare all things which need to be prepared
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// empty configuration
	auto yaml = YAML::Load("");
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// tasks is not a sequence
	yaml = YAML::Load("---\n"
					  "submission:\n"
					  "    job-id: 5\n"
					  "    language: cpp\n"
					  "    file-collector:\n"
					  "        hostname: localhost\n"
					  "        port: 36987\n"
					  "        username: 654321\n"
					  "        password: 123456\n"
					  "tasks: hello\n"
					  "...\n");
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

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
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// item job-id	missing in submission
	yaml = YAML::Load("---\n"
					  "submission:\n"
					  "    language: cpp\n"
					  "    file-collector:\n"
					  "        hostname: localhost\n"
					  "        port: 36987\n"
					  "        username: 654321\n"
					  "        password: 123456\n"
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
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// priority missing in internal task
	yaml = YAML::Load("---\n"
					  "submission:\n"
					  "    job-id: 5\n"
					  "    language: cpp\n"
					  "    file-collector:\n"
					  "        hostname: localhost\n"
					  "        port: 36987\n"
					  "        username: 654321\n"
					  "        password: 123456\n"
					  "tasks:\n"
					  "    - task-id: cp\n"
					  "      fatal-failure: true\n"
					  "      cmd:\n"
					  "          bin: cp\n"
					  "          args:\n"
					  "              - hello.cpp\n"
					  "              - hello_world.cpp\n"
					  "...\n");
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

#ifndef _WIN32
	// task-id missing in external task
	yaml = YAML::Load("---\n"
					  "submission:\n"
					  "    job-id: 5\n"
					  "    language: cpp\n"
					  "    file-collector:\n"
					  "        hostname: localhost\n"
					  "        port: 36987\n"
					  "        username: 654321\n"
					  "        password: 123456\n"
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
					  "                parallel: false\n"
					  "                disk-blocks: 50\n"
					  "                disk-inodes: 5\n"
					  "                environ-variable:\n"
					  "                    ISOLATE_BOX: /box\n"
					  "                    ISOLATE_TMP: /tmp\n"
					  "...\n");
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// correct configuration
	yaml = YAML::Load("---\n"
					  "submission:\n"
					  "    job-id: 5\n"
					  "    language: cpp\n"
					  "    file-collector:\n"
					  "        hostname: localhost\n"
					  "        port: 36987\n"
					  "        username: 654321\n"
					  "        password: 123456\n"
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
					  "                parallel: false\n"
					  "                disk-blocks: 50\n"
					  "                disk-inodes: 5\n"
					  "                environ-variable:\n"
					  "                    ISOLATE_BOX: /box\n"
					  "                    ISOLATE_TMP: /tmp\n"
					  "...\n");
	EXPECT_NO_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr));
#endif

	// cleanup
	remove_all(dir_root);
}

TEST(job_test, config_data) // TODO
{
	// prepare all things which need to be prepared
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// empty configuration
	auto yaml = YAML::Load("");
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// cleanup
	remove_all(dir_root);
}

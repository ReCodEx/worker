#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "../src/job/job.h"
#include "../src/fileman/cache_manager.h"
#include "../src/job/job_exception.h"


TEST(job_test, bad_parameters)
{
	std::shared_ptr<job_metadata> job_meta = std::make_shared<job_metadata>();
	std::shared_ptr<worker_config> worker_conf = std::make_shared<worker_config>();

	// job_config not given
	EXPECT_THROW(job(nullptr, nullptr, "", nullptr), job_exception);

	// worker_config not given
	EXPECT_THROW(job(job_meta, nullptr, "", nullptr), job_exception);

	// source path not exists
	EXPECT_THROW(job(job_meta, worker_conf, "", nullptr), job_exception);

	// fileman not given
	EXPECT_THROW(job(job_meta, worker_conf, temp_directory_path(), nullptr), job_exception);
}

TEST(job_test, bad_paths)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<worker_config>();
	auto fileman = std::make_shared<cache_manager>();

	// non-existing source code folder
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// source code path with no source codes in it
	create_directories(dir);
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// source code directory is not a directory
	dir = dir / "hello";
	std::ofstream hello(dir.string());
	hello << "hello" << std::endl;
	hello.close();
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, empty_submission_details)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<worker_config>();
	auto fileman = std::make_shared<cache_manager>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// job-id is empty
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// language is empty
	job_meta->job_id = "hello-job";
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// file-server-url is empty
	job_meta->language = "cpp";
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, empty_tasks_details)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<worker_config>();
	auto fileman = std::make_shared<cache_manager>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();
	job_meta->job_id = "hello-job";
	job_meta->language = "cpp";
	job_meta->file_server_url = "localhost";
	auto task = std::make_shared<task_metadata>();
	job_meta->tasks.push_back(task);

	// empty task-id
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// empty task priority
	task->task_id = "hello-task";
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// empty task binary
	task->priority = 1;
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// empty sandbox name
	task->binary = "hello";
	auto sandbox = std::make_shared<sandbox_config>();
	task->sandbox = sandbox;
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// non-defined hwgroup name
	sandbox->name = "isolate";
	EXPECT_THROW(job(job_meta, worker_conf, dir, fileman), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, non_empty_details)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto yaml = YAML::Load(
				"worker-id: 8\n"
				"broker-uri: localhost\n"
				"headers:\n"
				"    hwgroup: group1\n"
				"file-managers:\n"
				"    - hostname: http://localhost:80\n"
				"      username: 654321\n"
				"      password: 123456\n");
	auto worker_conf = std::make_shared<worker_config>(yaml);
	auto fileman = std::make_shared<cache_manager>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();
	job_meta->job_id = "hello-job";
	job_meta->language = "cpp";
	job_meta->file_server_url = "localhost";
	auto task = std::make_shared<task_metadata>();
	job_meta->tasks.push_back(task);
	task->task_id = "hello-task";
	task->priority = 1;
	task->binary = "hello";
	auto sandbox = std::make_shared<sandbox_config>();
	task->sandbox = sandbox;
	sandbox->name = "isolate";
	auto limits = std::make_shared<sandbox_limits>();
	sandbox->limits.emplace("group1", limits);

	// given correct (non empty) job/tasks details
	EXPECT_NO_THROW(job(job_meta, worker_conf, dir, fileman));

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, correct_build) // TODO
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<worker_config>();
	auto fileman = std::make_shared<cache_manager>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();
	job_meta->job_id = "hello-job";
	job_meta->language = "cpp";
	job_meta->file_server_url = "localhost";

	// non-existing source code folder
	EXPECT_NO_THROW(job(job_meta, worker_conf, dir, fileman));

	// cleanup after yourself
	remove_all(dir_root);
}

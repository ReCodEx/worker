#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>
#include <type_traits>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "../src/job/job.h"
#include "../src/job/job_exception.h"
#include "../src/fileman/cache_manager.h"
#include "../src/helpers/config.h"


TEST(job_test, bad_parameters)
{
	std::shared_ptr<job_metadata> job_meta = std::make_shared<job_metadata>();
	std::shared_ptr<worker_config> worker_conf = std::make_shared<worker_config>();
	auto fileman = std::make_shared<cache_manager>();

	// job_config not given
	EXPECT_THROW(job(nullptr, worker_conf, temp_directory_path(), temp_directory_path(), temp_directory_path(), fileman), job_exception);

	// worker_config not given
	EXPECT_THROW(job(job_meta, nullptr, temp_directory_path(), temp_directory_path(), temp_directory_path(), fileman), job_exception);

	// working dir not exists
	EXPECT_THROW(job(job_meta, worker_conf, "/recodex", temp_directory_path(), temp_directory_path(), fileman), job_exception);

	// source path not exists
	EXPECT_THROW(job(job_meta, worker_conf, temp_directory_path(), "/recodex", temp_directory_path(), fileman), job_exception);

	// result path not exists
	EXPECT_THROW(job(job_meta, worker_conf, temp_directory_path(), temp_directory_path(), "/recodex", fileman), job_exception);

	// fileman not given
	EXPECT_THROW(job(job_meta, worker_conf, temp_directory_path(), temp_directory_path(), temp_directory_path(), nullptr), job_exception);
}

TEST(job_test, bad_paths)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<worker_config>();
	auto fileman = std::make_shared<cache_manager>();

	// non-existing working directory
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// non-existing source code folder
	create_directories(dir_root);
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// source code path with no source codes in it
	create_directories(dir);
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// source code directory is not a directory
	dir = dir / "hello";
	std::ofstream hello(dir.string());
	hello << "hello" << std::endl;
	hello.close();
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

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
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// language is empty
	job_meta->job_id = "hello-job";
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// file-server-url is empty
	job_meta->language = "cpp";
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

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
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// empty task priority
	task->task_id = "hello-task";
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// empty task binary
	task->priority = 1;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// empty sandbox name
	task->binary = "hello";
	auto sandbox = std::make_shared<sandbox_config>();
	task->sandbox = sandbox;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// non-defined hwgroup name
	sandbox->name = "fake";
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, non_empty_details)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_yaml = YAML::Load(
				"---\n"
				"submission:\n"
				"    job-id: 5\n"
				"    language: cpp\n"
				"    file-collector: localhost\n"
				"tasks:\n"
				"    - task-id: eval\n"
				"      priority: 4\n"
				"      fatal-failure: false\n"
				"      cmd:\n"
				"          bin: recodex\n"
				"          args:\n"
				"              - -v\n"
				"              - \"-f 01.in\"\n"
				"      stdin: 01.in\n"
				"      stdout: 01.out\n"
				"      stderr: 01.err\n"
				"      sandbox:\n"
				"          name: fake\n"
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
				"                chdir: /eval\n"
				"                environ-variable:\n"
				"                    ISOLATE_BOX: /box\n"
				"                    ISOLATE_TMP: /tmp\n"
				"                bound-directories:\n"
				"                    /tmp/recodex: /recodex/tmp\n"
				"              - hw-group-id: group2\n"
				"...\n"
	);
	auto job_meta = helpers::build_job_metadata(job_yaml);
	auto default_yaml = YAML::Load(
				"worker-id: 8\n"
				"broker-uri: localhost\n"
				"headers:\n"
				"    hwgroup: group1\n"
				"file-managers:\n"
				"    - hostname: http://localhost:80\n"
				"      username: 654321\n"
				"      password: 123456\n"
	);
	auto worker_conf = std::make_shared<worker_config>(default_yaml);
	auto fileman = std::make_shared<cache_manager>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// given correct (non empty) job/tasks details
	EXPECT_NO_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman));

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, load_of_worker_defaults)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_yaml = YAML::Load(
				"---\n"
				"submission:\n"
				"    job-id: 5\n"
				"    language: cpp\n"
				"    file-collector: localhost\n"
				"tasks:\n"
				"    - task-id: eval\n"
				"      priority: 4\n"
				"      fatal-failure: false\n"
				"      cmd:\n"
				"          bin: recodex\n"
				"      sandbox:\n"
				"          name: fake\n"
				"          limits:\n"
				"              - hw-group-id: group1\n"
				"...\n"
	);
	auto job_meta = helpers::build_job_metadata(job_yaml);
	auto default_yaml = YAML::Load(
				"worker-id: 8\n"
				"broker-uri: localhost\n"
				"headers:\n"
				"    hwgroup: group1\n"
				"file-managers:\n"
				"    - hostname: http://localhost:80\n"
				"      username: 654321\n"
				"      password: 123456\n"
				"limits:\n"
				"    time: 5\n"
				"    wall-time: 6\n"
				"    extra-time: 2\n"
				"    stack-size: 50000\n"
				"    memory: 60000\n"
				"    parallel: 1\n"
				"    disk-size: 50\n"
				"    disk-files: 7\n"
	);
	auto worker_conf = std::make_shared<worker_config>(default_yaml);
	auto fileman = std::make_shared<cache_manager>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// construct and check
	job result(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman);

	ASSERT_EQ(result.get_task_queue().size(), 2u); // 2 because of fake_task as root
	auto task = result.get_task_queue().at(1);
	auto ext_task = std::dynamic_pointer_cast<external_task>(task);
	sandbox_limits limits = ext_task->get_limits();
	ASSERT_EQ(limits.cpu_time, 5);
	ASSERT_EQ(limits.wall_time, 6);
	ASSERT_EQ(limits.extra_time, 2);
	ASSERT_EQ(limits.stack_size, 50000u);
	ASSERT_EQ(limits.memory_usage, 60000u);
	ASSERT_EQ(limits.processes, 1u);
	ASSERT_EQ(limits.disk_size, 50u);
	ASSERT_EQ(limits.disk_files, 7u);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, correctly_built_queue)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_yaml = YAML::Load(
				"---\n"
				"submission:\n"
				"    job-id: 5\n"
				"    language: cpp\n"
				"    file-collector: localhost\n"
				"tasks:\n"
				"    - task-id: A\n"
				"      priority: 1\n"
				"      fatal-failure: false\n"
				"      cmd:\n"
				"          bin: mkdir\n"
				"          args:\n"
				"              - hello\n"
				"    - task-id: B\n"
				"      priority: 4\n"
				"      fatal-failure: false\n"
				"      dependencies:\n"
				"          - A\n"
				"      cmd:\n"
				"          bin: mkdir\n"
				"          args:\n"
				"              - hello\n"
				"    - task-id: C\n"
				"      priority: 6\n"
				"      fatal-failure: false\n"
				"      dependencies:\n"
				"          - B\n"
				"          - D\n"
				"      cmd:\n"
				"          bin: mkdir\n"
				"          args:\n"
				"              - hello\n"
				"    - task-id: D\n"
				"      priority: 2\n"
				"      fatal-failure: false\n"
				"      dependencies:\n"
				"          - A\n"
				"      cmd:\n"
				"          bin: mkdir\n"
				"          args:\n"
				"              - hello\n"
				"    - task-id: E\n"
				"      priority: 3\n"
				"      fatal-failure: false\n"
				"      dependencies:\n"
				"          - D\n"
				"      cmd:\n"
				"          bin: mkdir\n"
				"          args:\n"
				"              - hello\n"
				"    - task-id: F\n"
				"      priority: 5\n"
				"      fatal-failure: false\n"
				"      dependencies:\n"
				"          - D\n"
				"      cmd:\n"
				"          bin: mkdir\n"
				"          args:\n"
				"              - hello\n"
				"    - task-id: G\n"
				"      priority: 7\n"
				"      fatal-failure: false\n"
				"      dependencies:\n"
				"          - C\n"
				"      cmd:\n"
				"          bin: mkdir\n"
				"          args:\n"
				"              - hello\n"
				"...\n"
	);
	auto job_meta = helpers::build_job_metadata(job_yaml);
	auto default_yaml = YAML::Load(
				"worker-id: 8\n"
				"broker-uri: localhost\n"
				"headers:\n"
				"    hwgroup: group1\n"
				"file-managers:\n"
				"    - hostname: http://localhost:80\n"
				"      username: 654321\n"
				"      password: 123456\n"
				"limits:\n"
				"    time: 5\n"
				"    wall-time: 6\n"
				"    extra-time: 2\n"
				"    stack-size: 50000\n"
				"    memory: 60000\n"
				"    parallel: 1\n"
				"    disk-size: 50\n"
				"    disk-files: 7\n"
	);
	auto worker_conf = std::make_shared<worker_config>(default_yaml);
	auto fileman = std::make_shared<cache_manager>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// construct and check
	job result(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman);

	auto tasks = result.get_task_queue();
	ASSERT_EQ(tasks.size(), 8u); // +1 because of fake_task root
	ASSERT_EQ(tasks.at(0)->get_task_id(), ""); // fake root
	ASSERT_EQ(tasks.at(1)->get_task_id(), "A");
	ASSERT_EQ(tasks.at(2)->get_task_id(), "D");
	ASSERT_EQ(tasks.at(3)->get_task_id(), "E");
	ASSERT_EQ(tasks.at(4)->get_task_id(), "B");
	ASSERT_EQ(tasks.at(5)->get_task_id(), "F");
	ASSERT_EQ(tasks.at(6)->get_task_id(), "C");
	ASSERT_EQ(tasks.at(7)->get_task_id(), "G");

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, job_variables)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	path res_dir = dir_root / "job_test_results";
	auto job_yaml = YAML::Load(
				"---\n"
				"submission:\n"
				"    job-id: eval5\n"
				"    language: cpp\n"
				"    file-collector: localhost\n"
				"    log: false\n"
				"tasks:\n"
				"    - task-id: eval\n"
				"      priority: 4\n"
				"      fatal-failure: false\n"
				"      cmd:\n"
				"          bin: ${EVAL_DIR}/recodex\n"
				"          args:\n"
				"              - -v\n"
				"              - \"-f 01.in\"\n"
				"      stdin: before_stdin_${WORKER_ID}_after_stdin\n"
				"      stdout: before_stdout_${JOB_ID}_after_stdout\n"
				"      stderr: before_stderr_${RESULT_DIR}_after_stderr\n"
				"      sandbox:\n"
				"          name: fake\n"
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
				"                chdir: ${EVAL_DIR}\n"
				"                environ-variable:\n"
				"                    ISOLATE_BOX: /box\n"
				"                    ISOLATE_TMP: /tmp\n"
				"                bound-directories:\n"
				"                    - src: ${TEMP_DIR}" + std::string(1, path::preferred_separator) + "recodex\n"
				"                      dst: ${SOURCE_DIR}" + std::string(1, path::preferred_separator) + "tmp\n"
				"              - hw-group-id: group2\n"
				"...\n"
	);
	auto job_meta = helpers::build_job_metadata(job_yaml);
	auto default_yaml = YAML::Load(
				"worker-id: 8\n"
				"broker-uri: localhost\n"
				"headers:\n"
				"    hwgroup: group1\n"
				"file-managers:\n"
				"    - hostname: http://localhost:80\n"
				"      username: 654321\n"
				"      password: 123456\n"
	);
	auto worker_conf = std::make_shared<worker_config>(default_yaml);
	auto fileman = std::make_shared<cache_manager>();
	create_directories(dir);
	create_directories(res_dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// construct and check
	job j(job_meta, worker_conf, dir_root, dir, res_dir, fileman);
	ASSERT_EQ(j.get_task_queue().size(), 2u);

	auto task = j.get_task_queue().at(1);
	auto ext_task = std::dynamic_pointer_cast<external_task>(task);
	sandbox_limits limits = ext_task->get_limits();
	ASSERT_EQ(path(task->get_cmd()).string(), path("/evaluate/recodex").string());
	ASSERT_EQ(limits.std_input, "before_stdin_8_after_stdin");
	ASSERT_EQ(limits.std_output, "before_stdout_eval5_after_stdout");
	ASSERT_EQ(limits.std_error, "before_stderr_" + res_dir.string() + "_after_stderr");
	ASSERT_EQ(path(limits.chdir).string(), path("/evaluate").string());

	auto bnd_dirs = limits.bound_dirs;
	ASSERT_EQ(bnd_dirs.size(), 1u);
	ASSERT_EQ(path(std::get<0>(bnd_dirs[0])).string(), (temp_directory_path() / "recodex").string());
	ASSERT_EQ(path(std::get<1>(bnd_dirs[0])).string(), (dir / "tmp").string());

	// cleanup after yourself
	remove_all(dir_root);
}

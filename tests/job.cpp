#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>
#include <type_traits>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "mocks.h"
#include "../src/job/job.h"
#include "../src/job/job_exception.h"
#include "../src/helpers/config.h"

#include "../src/tasks/task_factory_interface.h"
#include "../src/tasks/external_task.h"
#include "../src/config/worker_config.h"


using namespace testing;

typedef std::tuple<std::string, std::string, sandbox_limits::dir_perm> mytuple;


bool operator==(const create_params &a, const create_params &b)
{
	// compare all but job logger
	return a.id == b.id && a.limits == b.limits && a.task_meta == b.task_meta && a.temp_dir == b.temp_dir &&
		a.worker_id == b.worker_id;
}

// get worker default limits
sandbox_limits get_default_limits()
{
	sandbox_limits default_limits;
	default_limits.cpu_time = 15;
	default_limits.wall_time = 16;
	default_limits.extra_time = 12;
	default_limits.stack_size = 150000;
	default_limits.memory_usage = 160000;
	default_limits.processes = 11;
	default_limits.disk_size = 150;
	default_limits.disk_files = 17;
	default_limits.environ_vars = {{"WORKER_CONFIG", "worker_config"}};
	default_limits.bound_dirs = {
		mytuple{"/tmp/recodex/worker_config", "/recodex/worker_config", sandbox_limits::dir_perm::RW}};
	return default_limits;
}

// get job metadata
std::shared_ptr<job_metadata> get_correct_meta()
{
	std::shared_ptr<job_metadata> job_meta = std::make_shared<job_metadata>();
	job_meta->job_id = "eval5";
	job_meta->file_server_url = "localhost";
	job_meta->hwgroups.push_back("group1");

	std::shared_ptr<task_metadata> task_meta = std::make_shared<task_metadata>();
	task_meta->task_id = "eval";
	task_meta->priority = 4;
	task_meta->fatal_failure = false;
	task_meta->binary = "recodex";
	task_meta->cmd_args = std::vector<std::string>{"-v", "-f 01.in"};
	std::shared_ptr<sandbox_config> sandbox_conf = std::make_shared<sandbox_config>();
	sandbox_conf->name = "fake";
	sandbox_conf->std_input = "01.in";
	sandbox_conf->std_output = "01.out";
	sandbox_conf->std_error = "01.err";

	// sandbox limits here are different than workers, so we can't call get_default_limits()
	std::shared_ptr<sandbox_limits> limits = std::make_shared<sandbox_limits>();
	limits->cpu_time = 5;
	limits->wall_time = 6;
	limits->extra_time = 7;
	limits->stack_size = 50000;
	limits->memory_usage = 60000;
	limits->processes = 1;
	limits->disk_size = 50;
	limits->disk_files = 10;
	limits->chdir = "/eval";
	limits->environ_vars = std::vector<std::pair<std::string, std::string>>{{"ISOLATE_BOX", "/box"}};
	limits->bound_dirs = std::vector<mytuple>{mytuple{"/tmp/recodex", "/recodex/tmp", sandbox_limits::dir_perm::RW}};

	sandbox_conf->loaded_limits = std::map<std::string, std::shared_ptr<sandbox_limits>>{{"group1", limits}};
	task_meta->sandbox = sandbox_conf;
	job_meta->tasks.push_back(task_meta);
	return job_meta;
}

// get job metadata with all (almost) default values.
// it's used for testing if worker default values are correctly used
std::shared_ptr<job_metadata> get_worker_default_meta()
{
	auto job_meta = get_correct_meta();
	job_meta->tasks[0]->sandbox->loaded_limits["group1"] = std::make_shared<sandbox_limits>();
	auto set_limits = job_meta->tasks[0]->sandbox->loaded_limits["group1"];
	set_limits->cpu_time = FLT_MAX;
	set_limits->wall_time = FLT_MAX;
	;
	set_limits->extra_time = FLT_MAX;
	set_limits->stack_size = SIZE_MAX;
	set_limits->memory_usage = SIZE_MAX;
	set_limits->processes = SIZE_MAX;
	set_limits->disk_size = SIZE_MAX;
	set_limits->disk_files = SIZE_MAX;
	set_limits->environ_vars = std::vector<std::pair<std::string, std::string>>{{"JOB_CONFIG", "job_config"}};
	set_limits->bound_dirs =
		std::vector<mytuple>{mytuple{"/tmp/recodex/job_config", "/recodex/job_config", sandbox_limits::dir_perm::RW}};

	return job_meta;
}

std::shared_ptr<task_metadata> get_simple_task(
	const std::string &name, size_t priority, const std::vector<std::string> &deps)
{
	std::shared_ptr<task_metadata> task = std::make_shared<task_metadata>();
	task->task_id = name;
	task->priority = priority;
	task->fatal_failure = false;
	task->dependencies = deps;
	task->binary = "mkdir";
	task->cmd_args = std::vector<std::string>{"-v"};
	task->type = task_type::EXECUTION;

	return task;
}

TEST(job_test, bad_parameters)
{
	std::shared_ptr<job_metadata> job_meta = std::make_shared<job_metadata>();
	std::shared_ptr<worker_config> worker_conf = std::make_shared<mock_worker_config>();
	auto factory = std::make_shared<mock_task_factory>();

	// job_config not given
	EXPECT_THROW(job(nullptr,
					 worker_conf,
					 temp_directory_path(),
					 temp_directory_path(),
					 temp_directory_path(),
					 factory,
					 nullptr),
		job_exception);

	// worker_config not given
	EXPECT_THROW(
		job(job_meta, nullptr, temp_directory_path(), temp_directory_path(), temp_directory_path(), factory, nullptr),
		job_exception);

	// working dir not exists
	EXPECT_THROW(job(job_meta, worker_conf, "/recodex", temp_directory_path(), temp_directory_path(), factory, nullptr),
		job_exception);

	// source path not exists
	EXPECT_THROW(job(job_meta, worker_conf, temp_directory_path(), "/recodex", temp_directory_path(), factory, nullptr),
		job_exception);

	// result path not exists
	EXPECT_THROW(job(job_meta, worker_conf, temp_directory_path(), temp_directory_path(), "/recodex", factory, nullptr),
		job_exception);

	// factory not given
	EXPECT_THROW(job(job_meta,
					 worker_conf,
					 temp_directory_path(),
					 temp_directory_path(),
					 temp_directory_path(),
					 nullptr,
					 nullptr),
		job_exception);
}

TEST(job_test, bad_paths)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<mock_worker_config>();
	auto factory = std::make_shared<mock_task_factory>();

	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));

	// non-existing working directory
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// non-existing source code folder
	create_directories(dir_root);
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// source code path with no source codes in it
	create_directories(dir);
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// source code directory is not a directory
	dir = dir / "hello";
	std::ofstream hello(dir.string());
	hello << "hello" << std::endl;
	hello.close();
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, empty_submission_details)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<mock_worker_config>();
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));

	auto factory = std::make_shared<mock_task_factory>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// job-id is empty
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// file-server-url is empty
	job_meta->job_id = "hello-job";
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// file-server-url is empty
	job_meta->file_server_url = "url://url.url";
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, empty_tasks_details)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<mock_worker_config>();
	auto factory = std::make_shared<mock_task_factory>();

	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));
	std::string hwgroup_ret = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(ReturnRef(hwgroup_ret));

	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	job_meta->job_id = "hello-job";
	job_meta->file_server_url = "localhost";
	job_meta->hwgroups.push_back("group1");
	auto task = std::make_shared<task_metadata>();
	job_meta->tasks.push_back(task);

	auto empty_task = std::make_shared<mock_task>();

	// empty task-id
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// empty task priority
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	task->task_id = "hello-task";
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// empty task binary
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	task->priority = 1;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// empty sandbox name
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	task->binary = "hello";
	auto sandbox = std::make_shared<sandbox_config>();
	task->sandbox = sandbox;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, non_empty_details)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";

	auto job_meta = get_correct_meta();
	auto worker_conf = std::make_shared<mock_worker_config>();
	auto factory = std::make_shared<mock_task_factory>();
	auto default_limits = get_default_limits();

	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(ReturnRef(default_limits));

	auto empty_task1 = std::make_shared<mock_task>();
	auto empty_task2 = std::make_shared<mock_task>(1, "eval");
	{
		InSequence s;
		EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task1));
		EXPECT_CALL((*factory), create_sandboxed_task(_)).WillRepeatedly(Return(empty_task2));
	}

	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// given correct (non empty) job/tasks details
	EXPECT_NO_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr));

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, load_of_worker_defaults)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto worker_conf = std::make_shared<mock_worker_config>();

	auto job_meta = get_worker_default_meta();

	auto default_limits = get_default_limits();
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(ReturnRef(default_limits));

	auto factory = std::make_shared<mock_task_factory>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// expect factory call to create sandboxed task
	// - important things are the pointers passed to factory
	// - after finish, we can read the values behind pointers and check changed values
	auto empty_task = std::make_shared<mock_task>();
	auto empty_task2 = std::make_shared<mock_task>(2, "eval");
	create_params params = {8,
		1,
		job_meta->tasks[0],
		job_meta->tasks[0]->sandbox,
		job_meta->tasks[0]->sandbox->loaded_limits["group1"],
		nullptr,
		dir_root.string()};
	{
		InSequence s;
		EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
		EXPECT_CALL((*factory), create_sandboxed_task(params)).WillOnce(Return(empty_task2));
	}

	// construct and check
	job result(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr);

	ASSERT_EQ(result.get_task_queue().size(), 1u); // 1 because of root_task as root is deleted

	// check changed values
	// Now it's not sure that the values are changed before create_sandboxed_task() is called.
	// But it should not matter (it's passed as pointer anyway), right values are there.
	std::shared_ptr<sandbox_limits> limits = params.limits;
	ASSERT_EQ(limits->cpu_time, 15);
	ASSERT_EQ(limits->wall_time, 16);
	ASSERT_EQ(limits->extra_time, 12);
	ASSERT_EQ(limits->stack_size, 150000u);
	ASSERT_EQ(limits->memory_usage, 160000u);
	ASSERT_EQ(limits->processes, 11u);
	ASSERT_EQ(limits->disk_size, 150u);
	ASSERT_EQ(limits->disk_files, 17u);

	std::vector<std::pair<std::string, std::string>> expected_environs;
	if (limits->environ_vars.at(0).first == "JOB_CONFIG") {
		expected_environs = {{"JOB_CONFIG", "job_config"}, {"WORKER_CONFIG", "worker_config"}};
	} else {
		expected_environs = {{"WORKER_CONFIG", "worker_config"}, {"JOB_CONFIG", "job_config"}};
	}
	std::vector<mytuple> expected_dirs = {
		mytuple{"/tmp/recodex/job_config", "/recodex/job_config", sandbox_limits::dir_perm::RW},
		mytuple{"/tmp/recodex/worker_config", "/recodex/worker_config", sandbox_limits::dir_perm::RW}};
	ASSERT_EQ(limits->environ_vars, expected_environs);
	ASSERT_EQ(limits->bound_dirs, expected_dirs);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, exceeded_worker_defaults)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto worker_conf = std::make_shared<mock_worker_config>();
	auto factory = std::make_shared<mock_task_factory>();

	auto default_limits = get_default_limits();
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(ReturnRef(default_limits));

	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	auto job_meta = get_correct_meta();
	auto empty_task = std::make_shared<mock_task>();

	// time exceeded worker defaults
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->cpu_time = 26;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// wall-time exceeded worker defaults
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->cpu_time = 6;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->wall_time = 27;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// extra-time exceeded worker defaults
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->wall_time = 2;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->extra_time = 23;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// stack-size exceeded worker defaults
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->extra_time = 3;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->stack_size = 260000;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// memory exceeded worker defaults
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->stack_size = 260;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->memory_usage = 270000;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// parallel exceeded worker defaults
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->memory_usage = 260;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->processes = 23;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// disk-size exceeded worker defaults
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->processes = 1;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->disk_size = 260;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// disk-files exceeded worker defaults
	EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->disk_size = 2;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->disk_files = 28;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, correctly_built_queue)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";

	auto job_meta = get_correct_meta();

	job_meta->tasks.clear();
	job_meta->tasks.push_back(get_simple_task("A", 1, {}));
	job_meta->tasks.push_back(get_simple_task("B", 4, {"A"}));
	job_meta->tasks.push_back(get_simple_task("C", 6, {"B", "D"}));
	job_meta->tasks.push_back(get_simple_task("D", 2, {"A"}));
	job_meta->tasks.push_back(get_simple_task("E", 3, {"D"}));
	job_meta->tasks.push_back(get_simple_task("F", 5, {"D"}));
	job_meta->tasks.push_back(get_simple_task("G", 7, {"C"}));

	auto worker_conf = std::make_shared<mock_worker_config>();
	auto default_limits = get_default_limits();
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(ReturnRef(default_limits));

	auto factory = std::make_shared<mock_task_factory>();
	std::vector<std::shared_ptr<mock_task>> mock_tasks;
	auto empty_task = std::make_shared<mock_task>();
	for (int i = 1; i < 8; i++) {
		mock_tasks.push_back(std::make_shared<mock_task>(i, job_meta->tasks[i - 1]));
	}
	{
		InSequence s;
		// expect root task to be created
		EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
		for (int i = 1; i < 8; i++) {
			// expect tasks A to G to be created
			EXPECT_CALL((*factory), create_internal_task(i, job_meta->tasks[i - 1]))
				.WillOnce(Return(mock_tasks[i - 1]));
		}
	}

	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// construct and check
	job result(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, nullptr);

	auto tasks = result.get_task_queue();
	ASSERT_EQ(tasks.size(), 7u);
	ASSERT_EQ(tasks.at(0)->get_task_id(), "A");
	ASSERT_EQ(tasks.at(1)->get_task_id(), "B");
	ASSERT_EQ(tasks.at(2)->get_task_id(), "D");
	ASSERT_EQ(tasks.at(3)->get_task_id(), "C");
	ASSERT_EQ(tasks.at(4)->get_task_id(), "G");
	ASSERT_EQ(tasks.at(5)->get_task_id(), "F");
	ASSERT_EQ(tasks.at(6)->get_task_id(), "E");

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, correctly_executed_job)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";

	auto job_meta = get_correct_meta();

	/*
	 * TASK TREE:
	 *
	 *      A
	 *     / \
	 *    B   D _
	 *     \ /  \\
	 *      C   E F
	 *             \
	 *              G
	 *
	 * Calling order: A, B, D, C, E, F, G
	 *
	 * progress_callback: F will fail, G will be skipped
	 */
	job_meta->tasks.clear();
	job_meta->tasks.push_back(get_simple_task("A", 1, {}));
	job_meta->tasks.push_back(get_simple_task("B", 4, {"A"}));
	job_meta->tasks.push_back(get_simple_task("C", 6, {"B", "D"}));
	job_meta->tasks.push_back(get_simple_task("D", 2, {"A"}));
	job_meta->tasks.push_back(get_simple_task("E", 5, {"D"}));
	job_meta->tasks.push_back(get_simple_task("F", 3, {"D"}));
	job_meta->tasks.push_back(get_simple_task("G", 7, {"F"}));

	size_t tasks_count = job_meta->tasks.size() + 1;

	auto worker_conf = std::make_shared<mock_worker_config>();
	auto default_limits = get_default_limits();
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(ReturnRef(default_limits));

	auto progress_callback = std::make_shared<mock_progress_callback>();
	auto factory = std::make_shared<mock_task_factory>();
	std::vector<std::shared_ptr<mock_task>> mock_tasks;
	auto empty_task = std::make_shared<mock_task>();
	auto empty_results = std::make_shared<task_results>();
	for (size_t i = 1; i < tasks_count; i++) {
		mock_tasks.push_back(std::make_shared<mock_task>(i, job_meta->tasks[i - 1]));
	}
	{
		InSequence s;
		// expect root task to be created
		EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));

		for (size_t i = 1; i < tasks_count; i++) {
			// expect tasks A to G to be created
			EXPECT_CALL((*factory), create_internal_task(i, job_meta->tasks[i - 1]))
				.WillOnce(Return(mock_tasks[i - 1]));
		}

		// progress callback calling expectations
		EXPECT_CALL(*progress_callback, job_started(_)).Times(1);
		EXPECT_CALL(*progress_callback, task_completed(_, _)).Times(tasks_count - 3);
		EXPECT_CALL(*progress_callback, task_failed(_, _)).Times(1);
		EXPECT_CALL(*progress_callback, task_skipped(_, _)).Times(1);
		EXPECT_CALL(*progress_callback, job_ended(_)).Times(1);
	}
	{ // ! out of sequence calling
		for (size_t i = 1; i < tasks_count - 2; i++) {
			// expect tasks A to G will be executed each at once
			EXPECT_CALL(*mock_tasks[i - 1], run()).WillOnce(Return(empty_results));
		}
		// task F will fail
		EXPECT_CALL(*mock_tasks[tasks_count - 3], run()).WillOnce(Throw(task_exception()));
	}

	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// construct
	job result(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, progress_callback);

	// and run it!...
	result.run();

	// cleanup after yourself
	remove_all(dir_root);
}

/**
 * Internal error means error in execution of inner task.
 * These errors can be possibly only "localy" place
 * and there is quite huge possibility that job can be executed on some other worker.
 */
TEST(job_test, internal_error_job)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";

	auto job_meta = get_correct_meta();

	/*
	 * TASK TREE:
	 *
	 *      A
	 *     / \
	 *    B   D
	 *     \ /
	 *      C
	 *
	 * Calling order: A, B, D, C
	 *
	 * progress_callback: D (inner) will fail and abort whole execution
	 */
	job_meta->tasks.clear();
	job_meta->tasks.push_back(get_simple_task("A", 1, {}));
	job_meta->tasks.push_back(get_simple_task("B", 4, {"A"}));
	job_meta->tasks.push_back(get_simple_task("C", 6, {"B", "D"}));
	job_meta->tasks.push_back(get_simple_task("D", 2, {"A"}));

	job_meta->tasks.back()->type = task_type::INNER;

	auto worker_conf = std::make_shared<mock_worker_config>();
	auto default_limits = get_default_limits();
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(ReturnRef(default_limits));

	auto progress_callback = std::make_shared<mock_progress_callback>();
	auto factory = std::make_shared<mock_task_factory>();
	std::vector<std::shared_ptr<mock_task>> mock_tasks;
	auto empty_task = std::make_shared<mock_task>();
	auto empty_results = std::make_shared<task_results>();
	for (size_t i = 0; i < job_meta->tasks.size(); i++) {
		mock_tasks.push_back(std::make_shared<mock_task>(i + 1, job_meta->tasks[i]));
	}

	{
		InSequence s;
		// expect root task to be created
		EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));

		for (size_t i = 0; i < mock_tasks.size(); i++) {
			// expect tasks to be created
			EXPECT_CALL((*factory), create_internal_task(i + 1, job_meta->tasks[i])).WillOnce(Return(mock_tasks[i]));
		}

		// progress callback calling expectations
		EXPECT_CALL(*progress_callback, job_started(_)).Times(1);
		EXPECT_CALL(*progress_callback, task_completed(_, _)).Times(2);

		// job aborted will not be sent because this function is called in job_evaluator
		// generelly after internal failure job wil immediatelly end
	}
	{
		InSequence s;
		EXPECT_CALL(*mock_tasks[0], run()).WillOnce(Return(empty_results));
		EXPECT_CALL(*mock_tasks[1], run()).WillOnce(Return(empty_results));
		// task D will fail
		EXPECT_CALL(*mock_tasks[3], run()).WillOnce(Throw(task_exception()));
	}

	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// construct
	job result(job_meta, worker_conf, dir_root, dir, temp_directory_path(), factory, progress_callback);

	// and run it!...
	EXPECT_THROW(result.run(), task_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, job_variables)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	path res_dir = dir_root / "job_test_results";
	auto worker_conf = std::make_shared<mock_worker_config>();
	auto factory = std::make_shared<mock_task_factory>();

	auto job_meta = get_correct_meta();
	job_meta->tasks[0]->binary = "${EVAL_DIR}/recodex";

	job_meta->tasks[0]->sandbox->std_input = "before_stdin_${WORKER_ID}_after_stdin";
	job_meta->tasks[0]->sandbox->std_output = "before_stdout_${JOB_ID}_after_stdout";
	job_meta->tasks[0]->sandbox->std_error = "before_stderr_${RESULT_DIR}_after_stderr";

	auto isolate_limits = job_meta->tasks[0]->sandbox->loaded_limits["group1"];
	isolate_limits->chdir = "${EVAL_DIR}";
	isolate_limits->bound_dirs =
		std::vector<mytuple>{mytuple{"${TEMP_DIR}" + std::string(1, path::preferred_separator) + "recodex",
			"${SOURCE_DIR}" + std::string(1, path::preferred_separator) + "tmp",
			sandbox_limits::dir_perm::RW}};

	auto default_limits = get_default_limits();
	default_limits.bound_dirs = {};
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(ReturnRef(default_limits));

	create_directories(dir);
	create_directories(res_dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// expect factory call to create sandboxed task
	// - important things are the pointers passed to factory
	// - after finish, we can read the values behind pointers and check changed values
	auto empty_task = std::make_shared<mock_task>();
	auto empty_task2 = std::make_shared<mock_task>(2, "eval");
	create_params params = {8,
		1,
		job_meta->tasks[0],
		job_meta->tasks[0]->sandbox,
		job_meta->tasks[0]->sandbox->loaded_limits["group1"],
		nullptr,
		dir_root.string()};
	{
		InSequence s;
		EXPECT_CALL((*factory), create_internal_task(0, _)).WillOnce(Return(empty_task));
		EXPECT_CALL((*factory), create_sandboxed_task(params)).WillOnce(Return(empty_task2));
	}

	// construct and check
	job j(job_meta, worker_conf, dir_root, dir, res_dir, factory, nullptr);
	ASSERT_EQ(j.get_task_queue().size(), 1u);

	std::shared_ptr<sandbox_limits> limits = params.limits;
	ASSERT_EQ(params.task_meta->binary, path("/evaluate/recodex").string());
	ASSERT_EQ(params.sandbox_conf->std_input, "before_stdin_8_after_stdin");
	ASSERT_EQ(params.sandbox_conf->std_output, "before_stdout_eval5_after_stdout");
	ASSERT_EQ(params.sandbox_conf->std_error, "before_stderr_" + res_dir.string() + "_after_stderr");
	ASSERT_EQ(path(limits->chdir).string(), path("/evaluate").string());

	auto bnd_dirs = limits->bound_dirs;
	ASSERT_EQ(bnd_dirs.size(), 1u);
	ASSERT_EQ(path(std::get<0>(bnd_dirs[0])).string(), (temp_directory_path() / "recodex").string());
	ASSERT_EQ(path(std::get<1>(bnd_dirs[0])).string(), (dir / "tmp").string());

	// cleanup after yourself
	remove_all(dir_root);
}

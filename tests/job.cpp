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
#include "../src/helpers/config.h"

#include "../src/fileman/file_manager_base.h"
#include "../src/config/worker_config.h"


typedef std::tuple<std::string, std::string, sandbox_limits::dir_perm> mytuple;

class mock_file_manager : public file_manager_base
{
public:
	mock_file_manager()	{}
	MOCK_CONST_METHOD0(get_caching_dir, std::string());
	MOCK_METHOD2(put_file, void(const std::string &name, const std::string &dst_path));
	MOCK_METHOD2(get_file, void(const std::string &src_name, const std::string &dst_path));
};

class mock_worker_config : public worker_config
{
public:
	mock_worker_config() {}
	virtual ~mock_worker_config() {}
	MOCK_CONST_METHOD0(get_hwgroup, const std::string &());
	MOCK_CONST_METHOD0(get_worker_id, size_t());
	MOCK_CONST_METHOD0(get_limits, const sandbox_limits &());
};

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
	default_limits.bound_dirs = { mytuple {"/tmp/recodex/worker_config", "/recodex/worker_config",
		sandbox_limits::dir_perm::RW}};
	return default_limits;
}

// get job metadata
std::shared_ptr<job_metadata> get_correct_meta()
{
	std::shared_ptr<job_metadata> job_meta = std::make_shared<job_metadata>();
	job_meta->job_id = "eval5";
	job_meta->file_server_url = "localhost";
	job_meta->language = "cpp";

	std::shared_ptr<task_metadata> task_meta = std::make_shared<task_metadata>();
	task_meta->task_id = "eval";
	task_meta->priority = 4;
	task_meta->fatal_failure = false;
	task_meta->binary = "recodex";
	task_meta->cmd_args = std::vector<std::string> {"-v", "-f 01.in"};
	task_meta->std_input = "01.in";
	task_meta->std_output = "01.out";
	task_meta->std_error = "01.err";
	std::shared_ptr<sandbox_config> sandbox_conf = std::make_shared<sandbox_config>();
	sandbox_conf->name = "fake";

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
	limits->environ_vars = std::vector<std::pair<std::string, std::string>> {{"ISOLATE_BOX", "/box"}};
	limits->bound_dirs = std::vector<mytuple> {
		mytuple {"/tmp/recodex", "/recodex/tmp", sandbox_limits::dir_perm::RW}
	};

	sandbox_conf->loaded_limits = std::map<std::string, std::shared_ptr<sandbox_limits>> {{"group1", limits}};
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
	set_limits->wall_time = FLT_MAX;;
	set_limits->extra_time = FLT_MAX;
	set_limits->stack_size = SIZE_MAX;
	set_limits->memory_usage = SIZE_MAX;
	set_limits->processes = SIZE_MAX;
	set_limits->disk_size = SIZE_MAX;
	set_limits->disk_files = SIZE_MAX;
	set_limits->environ_vars = std::vector<std::pair<std::string, std::string>> {{"JOB_CONFIG", "job_config"}};
	set_limits->bound_dirs = std::vector<mytuple> {
		mytuple {"/tmp/recodex/job_config",	"/recodex/job_config", sandbox_limits::dir_perm::RW}
	};

	return job_meta;
}

std::shared_ptr<task_metadata> get_simple_task(const std::string &name, size_t priority,
											   const std::vector<std::string> &deps)
{
	std::shared_ptr<task_metadata> task = std::make_shared<task_metadata>();
	task->task_id = name;
	task->priority = priority;
	task->fatal_failure = false;
	task->dependencies = deps;
	task->binary = "mkdir";
	task->cmd_args = std::vector<std::string> {"-v"};

	return task;
}


TEST(job_test, bad_parameters)
{
	std::shared_ptr<job_metadata> job_meta = std::make_shared<job_metadata>();
	std::shared_ptr<worker_config> worker_conf = std::make_shared<mock_worker_config>();
	auto fileman = std::make_shared<mock_file_manager>();

	// job_config not given
	EXPECT_THROW(
		job(nullptr, worker_conf, temp_directory_path(), temp_directory_path(), temp_directory_path(), fileman),
		job_exception);

	// worker_config not given
	EXPECT_THROW(job(job_meta, nullptr, temp_directory_path(), temp_directory_path(), temp_directory_path(), fileman),
		job_exception);

	// working dir not exists
	EXPECT_THROW(
		job(job_meta, worker_conf, "/recodex", temp_directory_path(), temp_directory_path(), fileman), job_exception);

	// source path not exists
	EXPECT_THROW(
		job(job_meta, worker_conf, temp_directory_path(), "/recodex", temp_directory_path(), fileman), job_exception);

	// result path not exists
	EXPECT_THROW(
		job(job_meta, worker_conf, temp_directory_path(), temp_directory_path(), "/recodex", fileman), job_exception);

	// fileman not given
	EXPECT_THROW(
		job(job_meta, worker_conf, temp_directory_path(), temp_directory_path(), temp_directory_path(), nullptr),
		job_exception);
}


TEST(job_test, bad_paths)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	auto job_meta = std::make_shared<job_metadata>();
	auto worker_conf = std::make_shared<mock_worker_config>();
	auto fileman = std::make_shared<mock_file_manager>();

	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(testing::Return(8));

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
	auto worker_conf = std::make_shared<mock_worker_config>();
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(testing::Return(8));

	auto fileman = std::make_shared<mock_file_manager>();
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
	auto worker_conf = std::make_shared<mock_worker_config>();
	auto fileman = std::make_shared<mock_file_manager>();

	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(testing::Return(8));
	std::string hwgroup_ret = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(testing::ReturnRef(hwgroup_ret));

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
	auto job_meta = get_correct_meta();
	auto worker_conf = std::make_shared<mock_worker_config>();
	auto fileman = std::make_shared<mock_file_manager>();
	auto default_limits = get_default_limits();

	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(testing::ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(testing::Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(testing::ReturnRef(default_limits));

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
	auto worker_conf = std::make_shared<mock_worker_config>();

	auto job_meta = get_worker_default_meta();

	auto default_limits = get_default_limits();
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(testing::ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(testing::Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(testing::ReturnRef(default_limits));

	auto fileman = std::make_shared<mock_file_manager>();
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// construct and check
	job result(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman);

	ASSERT_EQ(result.get_task_queue().size(), 2u); // 2 because of root_task as root
	auto task = result.get_task_queue().at(1);
	auto ext_task = std::dynamic_pointer_cast<external_task>(task);
	std::shared_ptr<sandbox_limits> limits = ext_task->get_limits();
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
		mytuple {"/tmp/recodex/job_config", "/recodex/job_config", sandbox_limits::dir_perm::RW},
		mytuple {"/tmp/recodex/worker_config", "/recodex/worker_config", sandbox_limits::dir_perm::RW}
	};
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
	auto fileman = std::make_shared<mock_file_manager>();

	auto default_limits = get_default_limits();
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(testing::ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(testing::Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(testing::ReturnRef(default_limits));

	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	auto job_meta = get_correct_meta();

	// time exceeded worker defaults
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->cpu_time = 26;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// wall-time exceeded worker defaults
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->cpu_time = 6;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->wall_time = 27;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// extra-time exceeded worker defaults
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->wall_time = 2;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->extra_time = 23;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// stack-size exceeded worker defaults
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->extra_time = 3;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->stack_size = 260000;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// memory exceeded worker defaults
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->stack_size = 260;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->memory_usage = 270000;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// parallel exceeded worker defaults
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->memory_usage = 260;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->processes = 23;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// disk-size exceeded worker defaults
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->processes = 1;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->disk_size = 260;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// disk-files exceeded worker defaults
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->disk_size = 2;
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->disk_files = 28;
	EXPECT_THROW(job(job_meta, worker_conf, dir_root, dir, temp_directory_path(), fileman), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}


TEST(job_test, correctly_built_queue)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";

	auto job_meta = get_correct_meta();

	job_meta->tasks[0] = get_simple_task("A", 1, {});
	job_meta->tasks.push_back(get_simple_task("B", 4, {"A"}));
	job_meta->tasks.push_back(get_simple_task("C", 6, {"B", "D"}));
	job_meta->tasks.push_back(get_simple_task("D", 2, {"A"}));
	job_meta->tasks.push_back(get_simple_task("E", 3, {"D"}));
	job_meta->tasks.push_back(get_simple_task("F", 5, {"D"}));
	job_meta->tasks.push_back(get_simple_task("G", 7, {"C"}));

	auto worker_conf = std::make_shared<mock_worker_config>();
	auto default_limits = get_default_limits();
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(testing::ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(testing::Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(testing::ReturnRef(default_limits));

	auto fileman = std::make_shared<mock_file_manager>();
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
	ASSERT_EQ(tasks.at(2)->get_task_id(), "B");
	ASSERT_EQ(tasks.at(3)->get_task_id(), "D");
	ASSERT_EQ(tasks.at(4)->get_task_id(), "C");
	ASSERT_EQ(tasks.at(5)->get_task_id(), "G");
	ASSERT_EQ(tasks.at(6)->get_task_id(), "F");
	ASSERT_EQ(tasks.at(7)->get_task_id(), "E");

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
	auto fileman = std::make_shared<mock_file_manager>();

	auto job_meta = get_correct_meta();
	job_meta->tasks[0]->binary = "${EVAL_DIR}/recodex";
	job_meta->tasks[0]->std_input = "before_stdin_${WORKER_ID}_after_stdin";
	job_meta->tasks[0]->std_output = "before_stdout_${JOB_ID}_after_stdout";
	job_meta->tasks[0]->std_error = "before_stderr_${RESULT_DIR}_after_stderr";
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->chdir = "${EVAL_DIR}";
	job_meta->tasks[0]->sandbox->loaded_limits["group1"]->bound_dirs = std::vector<mytuple> {
		mytuple {"${TEMP_DIR}" + std::string(1, path::preferred_separator) + "recodex",
				 "${SOURCE_DIR}" + std::string(1, path::preferred_separator) + "tmp",
				 sandbox_limits::dir_perm::RW}
		};

	auto default_limits = get_default_limits();
	default_limits.bound_dirs = {};
	std::string group_name = "group1";
	EXPECT_CALL((*worker_conf), get_hwgroup()).WillRepeatedly(testing::ReturnRef(group_name));
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(testing::Return(8));
	EXPECT_CALL((*worker_conf), get_limits()).WillRepeatedly(testing::ReturnRef(default_limits));

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
	std::shared_ptr<sandbox_limits> limits = ext_task->get_limits();
	ASSERT_EQ(path(task->get_cmd()).string(), path("/evaluate/recodex").string());
	ASSERT_EQ(limits->std_input, "before_stdin_8_after_stdin");
	ASSERT_EQ(limits->std_output, "before_stdout_eval5_after_stdout");
	ASSERT_EQ(limits->std_error, "before_stderr_" + res_dir.string() + "_after_stderr");
	ASSERT_EQ(path(limits->chdir).string(), path("/evaluate").string());

	auto bnd_dirs = limits->bound_dirs;
	ASSERT_EQ(bnd_dirs.size(), 1u);
	ASSERT_EQ(path(std::get<0>(bnd_dirs[0])).string(), (temp_directory_path() / "recodex").string());
	ASSERT_EQ(path(std::get<1>(bnd_dirs[0])).string(), (dir / "tmp").string());

	// cleanup after yourself
	remove_all(dir_root);
}

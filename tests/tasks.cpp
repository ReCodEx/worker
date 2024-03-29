#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstdio>

#include "tasks/internal/archivate_task.h"
#include "tasks/internal/cp_task.h"
#include "tasks/internal/extract_task.h"
#include "tasks/internal/mkdir_task.h"
#include "tasks/internal/rename_task.h"
#include "tasks/internal/rm_task.h"
#include "tasks/internal/fetch_task.h"
#include "tasks/internal/exists_task.h"
#include "tasks/external_task.h"
#include "tasks/root_task.h"
#include "tasks/task_factory.h"
#include "tasks/create_params.h"
#include "config/sandbox_config.h"
#include "mocks.h"

namespace fs = std::filesystem;


std::shared_ptr<task_metadata> get_task_meta()
{
	auto res = std::make_shared<task_metadata>();
	res->task_id = "id2";
	res->priority = 3;
	res->fatal_failure = false;
	res->dependencies = {"dep1", "dep2", "dep3"};
	res->binary = "command";
	res->cmd_args = {"arg1", "arg2"};
	res->sandbox = std::make_shared<sandbox_config>();
	res->sandbox->loaded_limits.insert(
		std::pair<std::string, std::shared_ptr<sandbox_limits>>("group1", std::make_shared<sandbox_limits>()));
	return res;
}

std::shared_ptr<task_metadata> get_three_args()
{
	auto res = get_task_meta();
	res->cmd_args = {"one", "two", "three"};
	return res;
}

std::shared_ptr<task_metadata> get_two_args()
{
	auto res = get_task_meta();
	res->cmd_args = {"one", "two"};
	return res;
}

std::shared_ptr<task_metadata> get_one_args()
{
	auto res = get_task_meta();
	res->cmd_args = {"one"};
	return res;
}

std::shared_ptr<task_metadata> get_zero_args()
{
	auto res = get_task_meta();
	res->cmd_args = {};
	return res;
}

TEST(Tasks, InternalArchivateTask)
{
	EXPECT_THROW(archivate_task(1, get_three_args()), task_exception);
	EXPECT_THROW(archivate_task(1, get_one_args()), task_exception);
	EXPECT_THROW(archivate_task(1, get_zero_args()), task_exception);
	EXPECT_NO_THROW(archivate_task(1, get_two_args()));
}

TEST(Tasks, InternalCpTask)
{
	EXPECT_THROW(cp_task(1, get_three_args()), task_exception);
	EXPECT_THROW(cp_task(1, get_one_args()), task_exception);
	EXPECT_THROW(cp_task(1, get_zero_args()), task_exception);
	EXPECT_NO_THROW(cp_task(1, get_two_args()));
}

TEST(Tasks, InternalCpTaskExceptionBug)
{
	auto meta = get_task_meta();
	meta->cmd_args = {"/proc/self/cmdline", "/nonexistingdir/a"};
	std::shared_ptr<task_results> res;
	EXPECT_NO_THROW(res = cp_task(1, meta).run());
	EXPECT_EQ(task_status::FAILED, res->status);
}

TEST(Tasks, InternalCpTaskWildcards)
{
	// prepare directories and files
	auto temp = fs::temp_directory_path();
	auto src_dir = temp / std::string(std::tmpnam(nullptr));
	auto dst_dir = temp / std::string(std::tmpnam(nullptr));
	fs::create_directories(src_dir);
	fs::create_directories(dst_dir);
	for (const auto &i : {"a1", "a2", "b1"}) {
		std::ofstream ofs((src_dir / i).string());
		ofs << i;
	}

	// run the test and check asserts
	auto meta = get_task_meta();
	meta->cmd_args = {src_dir.string() + "/a*", dst_dir.string()};
	std::shared_ptr<task_results> res;
	EXPECT_NO_THROW(res = cp_task(1, meta).run());
	EXPECT_EQ(task_status::OK, res->status);
	EXPECT_TRUE(fs::is_regular_file(dst_dir / "a1"));
	EXPECT_TRUE(fs::is_regular_file(dst_dir / "a2"));
	EXPECT_FALSE(fs::is_regular_file(dst_dir / "b1"));

	// cleanup
	fs::remove_all(src_dir);
	fs::remove_all(dst_dir);
}

TEST(Tasks, InternalExtractTask)
{
	EXPECT_THROW(extract_task(1, get_three_args()), task_exception);
	EXPECT_THROW(extract_task(1, get_one_args()), task_exception);
	EXPECT_THROW(extract_task(1, get_zero_args()), task_exception);
	EXPECT_NO_THROW(extract_task(1, get_two_args()));
}

TEST(Tasks, InternalMkdirTask)
{
	EXPECT_NO_THROW(mkdir_task(1, get_three_args()));
	EXPECT_NO_THROW(mkdir_task(1, get_one_args()));
	EXPECT_THROW(mkdir_task(1, get_zero_args()), task_exception);
	EXPECT_NO_THROW(mkdir_task(1, get_two_args()));
}

TEST(Tasks, InternalRenameTask)
{
	EXPECT_THROW(rename_task(1, get_three_args()), task_exception);
	EXPECT_THROW(rename_task(1, get_one_args()), task_exception);
	EXPECT_THROW(rename_task(1, get_zero_args()), task_exception);
	EXPECT_NO_THROW(rename_task(1, get_two_args()));
}

TEST(Tasks, InternalRmTask)
{
	EXPECT_NO_THROW(rm_task(1, get_three_args()));
	EXPECT_NO_THROW(rm_task(1, get_one_args()));
	EXPECT_THROW(rm_task(1, get_zero_args()), task_exception);
	EXPECT_NO_THROW(rm_task(1, get_two_args()));
}

TEST(Tasks, InternalFetchTask)
{
	EXPECT_THROW(fetch_task(1, get_three_args(), nullptr), task_exception);
	EXPECT_THROW(fetch_task(1, get_one_args(), nullptr), task_exception);
	EXPECT_THROW(fetch_task(1, get_zero_args(), nullptr), task_exception);
	EXPECT_NO_THROW(fetch_task(1, get_two_args(), nullptr));
}

TEST(Tasks, InternalExistsTask)
{
	EXPECT_THROW(exists_task(1, get_zero_args()), task_exception);
	EXPECT_THROW(exists_task(1, get_one_args()), task_exception);
	EXPECT_NO_THROW(exists_task(1, get_two_args()));
	EXPECT_NO_THROW(exists_task(1, get_three_args()));
}


class test_task_base : public task_base
{
public:
	test_task_base(std::size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
	{
	}
	virtual ~test_task_base()
	{
	}
	std::shared_ptr<task_results> run()
	{
		return nullptr;
	}
};

TEST(Tasks, TaskBase)
{
	test_task_base base(1, get_task_meta());
	EXPECT_EQ(base.get_cmd(), "command");
	std::vector<std::string> dep{"dep1", "dep2", "dep3"};
	EXPECT_EQ(base.get_dependencies(), dep);
	EXPECT_EQ(base.get_fatal_failure(), false);
	EXPECT_EQ(base.get_id(), static_cast<std::size_t>(1));
	EXPECT_EQ(base.get_priority(), static_cast<std::size_t>(3));
	EXPECT_EQ(base.get_task_id(), "id2");
	EXPECT_TRUE(base.get_children().empty());
	auto children = std::shared_ptr<task_base>(new test_task_base(2, get_task_meta()));
	base.add_children(children);
	EXPECT_EQ(base.get_children()[0]->get_task_id(), children->get_task_id());
}

TEST(Tasks, TaskFactory)
{
	task_factory factory(nullptr);
	auto meta = get_task_meta();
	std::shared_ptr<task_base> task;

	// archivate task
	meta->binary = "archivate";
	task = factory.create_internal_task(0, meta);
	EXPECT_NE(std::dynamic_pointer_cast<archivate_task>(task), nullptr);

	// cp task
	meta->binary = "cp";
	task = factory.create_internal_task(0, meta);
	EXPECT_NE(std::dynamic_pointer_cast<cp_task>(task), nullptr);

	// extract task
	meta->binary = "extract";
	task = factory.create_internal_task(0, meta);
	EXPECT_NE(std::dynamic_pointer_cast<extract_task>(task), nullptr);

	// fetch task
	meta->binary = "fetch";
	task = factory.create_internal_task(0, meta);
	EXPECT_NE(std::dynamic_pointer_cast<fetch_task>(task), nullptr);

	// mkdir task
	meta->binary = "mkdir";
	task = factory.create_internal_task(0, meta);
	EXPECT_NE(std::dynamic_pointer_cast<mkdir_task>(task), nullptr);

	// rename task
	meta->binary = "rename";
	task = factory.create_internal_task(0, meta);
	EXPECT_NE(std::dynamic_pointer_cast<rename_task>(task), nullptr);

	// rm task
	meta->binary = "rm";
	task = factory.create_internal_task(0, meta);
	EXPECT_NE(std::dynamic_pointer_cast<rm_task>(task), nullptr);

	// root task
	// - with explicit nullptr argument
	meta->binary = "archivate";
	task = factory.create_internal_task(0, nullptr);
	EXPECT_NE(std::dynamic_pointer_cast<root_task>(task), nullptr);
	// - without explicit meta argument
	task = factory.create_internal_task(0);
	EXPECT_NE(std::dynamic_pointer_cast<root_task>(task), nullptr);

	// unknown internal task
	meta->binary = "unknown_internal_bianry";
	task = factory.create_internal_task(0, meta);
	EXPECT_EQ(task, nullptr);

	// worker config
	auto worker_conf = std::make_shared<mock_worker_config>();
	EXPECT_CALL((*worker_conf), get_worker_id()).WillRepeatedly(Return(8));

	// external task
	meta->binary = "external_command";
	create_params params = {worker_conf, 1, meta, meta->sandbox->loaded_limits["group1"], nullptr, "", fs::path()};
	EXPECT_THROW(factory.create_sandboxed_task(params), task_exception); // Sandbox config is nullptr
	meta->sandbox = std::make_shared<sandbox_config>();
	meta->sandbox->name = "whatever_sandbox";
	EXPECT_THROW(factory.create_sandboxed_task(params), task_exception); // Unknown sandbox type
// Isolate only supported on linux platform
#ifndef _WIN32
	meta->sandbox->name = "isolate";
	task = factory.create_sandboxed_task(params);
	EXPECT_NE(std::dynamic_pointer_cast<external_task>(task), nullptr);
#endif
}

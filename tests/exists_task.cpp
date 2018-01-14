#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <memory>
#include "../src/tasks/internal/exists_task.h"

namespace fs = boost::filesystem;

class exists_task_test : public ::testing::Test
{
protected:
	fs::path root;
	fs::path target;
	std::shared_ptr<exists_task> task;
	std::shared_ptr<task_metadata> task_meta;

	virtual void SetUp()
	{
		root = fs::temp_directory_path() / fs::unique_path();
		fs::create_directory(root);
		fs::create_directory(root / "subdir");

		create_file(root / "file_a");
		create_file(root / "subdir" / "file_b");

		task_meta = std::make_shared<task_metadata>();
		task_meta->cmd_args = {"failure message", root.string()};
		task = std::make_shared<exists_task>(1, task_meta);
	}

	virtual void TearDown()
	{
		fs::remove_all(root);
		fs::remove(root);
	}

	void create_file(const fs::path &path)
	{
		std::ofstream f(path.string());
		f << "test file";
		f.close();
	}
};

TEST_F(exists_task_test, root_exists)
{
	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;
}

TEST_F(exists_task_test, file_subdir_exists)
{
	task_meta->cmd_args = {"failure message", (root / "subdir").string(), (root / "file_a").string()};

	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;
}

TEST_F(exists_task_test, file_not_exists)
{
	task_meta->cmd_args = {"failure message", (root / "not_a_file").string()};

	auto results = task->run();
	ASSERT_EQ(task_status::FAILED, results->status);
	ASSERT_TRUE(results->error_message.find("cannot be found") != std::string::npos);
	ASSERT_EQ("failure message", results->output_stderr);
}

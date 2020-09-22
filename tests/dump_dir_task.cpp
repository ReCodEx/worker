#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <memory>
#include "tasks/internal/dump_dir_task.h"

namespace fs = boost::filesystem;

class dump_dir_task_test : public ::testing::Test
{
protected:
	fs::path root;
	fs::path target;
	std::shared_ptr<dump_dir_task> task;
	std::shared_ptr<task_metadata> task_meta;

	virtual void SetUp()
	{
		target = fs::temp_directory_path() / fs::unique_path();
		fs::create_directory(target);

		root = fs::temp_directory_path() / fs::unique_path();
		fs::create_directory(root);
		fs::create_directory(root / "subdir");

		create_file(root / "file_a", 2048);
		create_file(root / "file_b", 4096);
		create_file(root / "subdir" / "file_c", 1536);

		task_meta = std::make_shared<task_metadata>();
		task_meta->cmd_args = {root.string(), (target / "dir").string(), "16384"};
		task = std::make_shared<dump_dir_task>(1, task_meta);
	}

	virtual void TearDown()
	{
		fs::remove_all(root);
		fs::remove(root);
		fs::remove_all(target);
		fs::remove(target);
	}

	void create_file(const fs::path &path, std::size_t size)
	{
		std::ofstream f(path.string());
		for (std::size_t i = 0; i < size; i++) { f << "a"; }
		f.close();
	}
};

TEST_F(dump_dir_task_test, everything_fits)
{
	// setup command line arguments
	task_meta->cmd_args = {root.string(), (target / "dir").string(), "16384"};

	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;

	ASSERT_TRUE(fs::exists(target / "dir" / "file_a"));
	ASSERT_TRUE(fs::exists(target / "dir" / "file_b"));
	ASSERT_TRUE(fs::exists(target / "dir" / "subdir" / "file_c"));
}

TEST_F(dump_dir_task_test, everything_skipped)
{
	// setup command line arguments
	task_meta->cmd_args = {root.string(), (target / "dir").string(), "1"};

	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;

	ASSERT_FALSE(fs::exists(target / "dir" / "file_a"));
	ASSERT_TRUE(fs::exists(target / "dir" / "file_a.skipped"));
	ASSERT_FALSE(fs::exists(target / "dir" / "file_b"));
	ASSERT_TRUE(fs::exists(target / "dir" / "file_b.skipped"));
	ASSERT_FALSE(fs::exists(target / "dir" / "subdir" / "file_c"));
	ASSERT_TRUE(fs::exists(target / "dir" / "subdir" / "file_c.skipped"));
}

TEST_F(dump_dir_task_test, largest_skipped)
{
	// setup command line arguments
	task_meta->cmd_args = {root.string(), (target / "dir").string(), "4"};

	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;

	ASSERT_TRUE(fs::exists(target / "dir" / "file_a"));
	ASSERT_FALSE(fs::exists(target / "dir" / "file_b"));
	ASSERT_TRUE(fs::exists(target / "dir" / "file_b.skipped"));
	ASSERT_TRUE(fs::exists(target / "dir" / "subdir" / "file_c"));
}

TEST_F(dump_dir_task_test, nothing_excluded)
{
	// setup command line arguments
	task_meta->cmd_args = {root.string(), (target / "dir").string(), "16384", "nonexisting-a", "nonexisting-b"};

	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;

	ASSERT_TRUE(fs::exists(target / "dir" / "file_a"));
	ASSERT_TRUE(fs::exists(target / "dir" / "file_b"));
	ASSERT_TRUE(fs::exists(target / "dir" / "subdir" / "file_c"));
}

TEST_F(dump_dir_task_test, exclude_single_file)
{
	// setup command line arguments
	task_meta->cmd_args = {root.string(), (target / "dir").string(), "16384", "file_a"};

	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;

	ASSERT_FALSE(fs::exists(target / "dir" / "file_a"));
	ASSERT_TRUE(fs::exists(target / "dir" / "file_b"));
	ASSERT_TRUE(fs::exists(target / "dir" / "subdir" / "file_c"));
}

TEST_F(dump_dir_task_test, exclude_subdir)
{
	// setup command line arguments
	task_meta->cmd_args = {root.string(), (target / "dir").string(), "16384", (fs::path("subdir") / "file*").string()};

	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;

	ASSERT_TRUE(fs::exists(target / "dir" / "file_a"));
	ASSERT_TRUE(fs::exists(target / "dir" / "file_b"));
	ASSERT_FALSE(fs::exists(target / "dir" / "subdir" / "file_c"));
}

TEST_F(dump_dir_task_test, exclude_root_dir_files)
{
	// setup command line arguments
	task_meta->cmd_args = {root.string(), (target / "dir").string(), "16384", "file*"};

	auto results = task->run();
	ASSERT_EQ(task_status::OK, results->status) << "Failed with: " + results->error_message;

	ASSERT_FALSE(fs::exists(target / "dir" / "file_a"));
	ASSERT_FALSE(fs::exists(target / "dir" / "file_b"));
	ASSERT_TRUE(fs::exists(target / "dir" / "subdir" / "file_c"));
}

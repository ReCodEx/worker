#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <memory>
#include "tasks/internal/truncate_task.h"

namespace fs = boost::filesystem;

class truncate_task_test : public ::testing::Test
{
protected:
	fs::path root;
	std::shared_ptr<truncate_task> task;
	std::shared_ptr<task_metadata> task_meta;

	virtual void SetUp()
	{
		root = fs::temp_directory_path() / fs::unique_path();
		fs::create_directory(root);

		create_file(root / "file_a", 16384);

		task_meta = std::make_shared<task_metadata>();
		task_meta->cmd_args = {(root / "file_a").string(), "16384"};
		task = std::make_shared<truncate_task>(1, task_meta);
	}

	virtual void TearDown()
	{
		fs::remove_all(root);
		fs::remove(root);
	}

	void create_file(const fs::path &path, std::size_t size)
	{
		std::ofstream f(path.string());
		for (std::size_t i = 0; i < size; i++) { f << "a"; }
		f.close();
	}
};

TEST_F(truncate_task_test, truncate)
{
	task_meta->cmd_args[1] = "2";
	task->run();

	ASSERT_EQ(2048u, fs::file_size(root / "file_a"));
}

TEST_F(truncate_task_test, small_file)
{
	task_meta->cmd_args[1] = "32";
	task->run();

	ASSERT_EQ(16384u, fs::file_size(root / "file_a"));
}

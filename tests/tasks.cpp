#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/tasks/internal/archivate_task.h"
#include "../src/tasks/internal/cp_task.h"
#include "../src/tasks/internal/extract_task.h"
#include "../src/tasks/internal/mkdir_task.h"
#include "../src/tasks/internal/rename_task.h"
#include "../src/tasks/internal/rm_task.h"
#include "../src/tasks/internal/fetch_task.h"


std::shared_ptr<task_metadata> get_task_meta()
{
	auto res = std::make_shared<task_metadata>();
	res->task_id = "id2";
	res->priority = 3;
	res->fatal_failure = false;
	res->dependencies = {"dep1", "dep2", "dep3"};
	res->binary = "command";
	res->cmd_args = {"arg1", "arg2"};
	res->std_input = "";
	res->std_output = "";
	res->std_error = "";
	res->sandbox = nullptr;
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


class test_task_base : public task_base
{
public:
	test_task_base(size_t id, std::shared_ptr<task_metadata> task_meta) : task_base(id, task_meta)
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
	EXPECT_EQ(base.get_id(), static_cast<size_t>(1));
	EXPECT_EQ(base.get_priority(), static_cast<size_t>(3));
	EXPECT_EQ(base.get_task_id(), "id2");
	EXPECT_TRUE(base.get_children().empty());
	auto children = std::shared_ptr<task_base>(new test_task_base(2, get_task_meta()));
	base.add_children(children);
	EXPECT_EQ(base.get_children()[0]->get_task_id(), children->get_task_id());
}

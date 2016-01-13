#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/tasks/internal/archivate_task.h"
#include "../src/tasks/internal/cp_task.h"
#include "../src/tasks/internal/extract_task.h"
#include "../src/tasks/internal/mkdir_task.h"
#include "../src/tasks/internal/rename_task.h"
#include "../src/tasks/internal/rm_task.h"
#include "../src/tasks/internal/fetch_task.h"


TEST(Tasks, InternalArchivateTask)
{
	EXPECT_THROW(archivate_task(1, "2", 3, false, "", {"one", "two", "three"}, {}), task_exception);
	EXPECT_THROW(archivate_task(1, "2", 3, false, "", {"one"}, {}), task_exception);
	EXPECT_THROW(archivate_task(1, "2", 3, false, "", {}, {}), task_exception);
	EXPECT_NO_THROW(archivate_task(1, "2", 3, false, "", {"one", "two"}, {}));
}

TEST(Tasks, InternalCpTask)
{
	EXPECT_THROW(cp_task(1, "2", 3, false, "", {"one", "two", "three"}, {}), task_exception);
	EXPECT_THROW(cp_task(1, "2", 3, false, "", {"one"}, {}), task_exception);
	EXPECT_THROW(cp_task(1, "2", 3, false, "", {}, {}), task_exception);
	EXPECT_NO_THROW(cp_task(1, "2", 3, false, "", {"one", "two"}, {}));
}

TEST(Tasks, InternalExtractTask)
{
	EXPECT_THROW(extract_task(1, "2", 3, false, "", {"one", "two", "three"}, {}), task_exception);
	EXPECT_THROW(extract_task(1, "2", 3, false, "", {"one"}, {}), task_exception);
	EXPECT_THROW(extract_task(1, "2", 3, false, "", {}, {}), task_exception);
	EXPECT_NO_THROW(extract_task(1, "2", 3, false, "", {"one", "two"}, {}));
}

TEST(Tasks, InternalMkdirTask)
{
	EXPECT_NO_THROW(mkdir_task(1, "2", 3, false, "", {"one", "two", "three"}, {}));
	EXPECT_NO_THROW(mkdir_task(1, "2", 3, false, "", {"one"}, {}));
	EXPECT_THROW(mkdir_task(1, "2", 3, false, "", {}, {}), task_exception);
	EXPECT_NO_THROW(mkdir_task(1, "2", 3, false, "", {"one", "two"}, {}));
}

TEST(Tasks, InternalRenameTask)
{
	EXPECT_THROW(rename_task(1, "2", 3, false, "", {"one", "two", "three"}, {}), task_exception);
	EXPECT_THROW(rename_task(1, "2", 3, false, "", {"one"}, {}), task_exception);
	EXPECT_THROW(rename_task(1, "2", 3, false, "", {}, {}), task_exception);
	EXPECT_NO_THROW(rename_task(1, "2", 3, false, "", {"one", "two"}, {}));
}

TEST(Tasks, InternalRmTask)
{
	EXPECT_NO_THROW(rm_task(1, "2", 3, false, "", {"one", "two", "three"}, {}));
	EXPECT_NO_THROW(rm_task(1, "2", 3, false, "", {"one"}, {}));
	EXPECT_THROW(rm_task(1, "2", 3, false, "", {}, {}), task_exception);
	EXPECT_NO_THROW(rm_task(1, "2", 3, false, "", {"one", "two"}, {}));
}

TEST(Tasks, InternalFetchTask)
{
	EXPECT_THROW(fetch_task(1, "2", 3, false, "", {"one", "two", "three"}, {}, nullptr), task_exception);
	EXPECT_THROW(fetch_task(1, "2", 3, false, "", {"one"}, {}, nullptr), task_exception);
	EXPECT_THROW(fetch_task(1, "2", 3, false, "", {}, {}, nullptr), task_exception);
	EXPECT_NO_THROW(fetch_task(1, "2", 3, false, "", {"one", "two"}, {}, nullptr));
}


class test_task_base : public task_base {
public:
	test_task_base(size_t id, std::string task_id, size_t priority, bool fatal,
				   const std::vector<std::string> &dependencies,
				   const std::string &cmd, const std::vector<std::string> &arguments) :
		task_base(id, task_id, priority, fatal, dependencies, cmd, arguments) {}
	virtual ~test_task_base() {}
	void run() {}
};

TEST(Tasks, TaskBase)
{
	test_task_base base(1, "id2", 3, false, {"dep1", "dep2", "dep3"}, "command", {"arg1", "arg2"});
	EXPECT_EQ(base.get_cmd(), "command");
	std::vector<std::string> dep{"dep1", "dep2", "dep3"};
	EXPECT_EQ(base.get_dependencies(), dep);
	EXPECT_EQ(base.get_fatal_failure(), false);
	EXPECT_EQ(base.get_id(), static_cast<size_t>(1));
	EXPECT_EQ(base.get_priority(), static_cast<size_t>(3));
	EXPECT_EQ(base.get_task_id(), "id2");
	EXPECT_TRUE(base.get_children().empty());
	auto children = std::shared_ptr<task_base>(new test_task_base(2, "id3", 4, true, {}, "", {}));
	base.add_children(children);
	EXPECT_EQ(base.get_children()[0]->get_task_id(), children->get_task_id());
}

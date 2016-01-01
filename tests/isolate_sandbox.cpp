#ifndef _WIN32

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "../src/sandbox/isolate_sandbox.h"

TEST(IsolateSandbox, BasicCreation)
{
	sandbox_limits limits;
	EXPECT_NO_THROW(isolate_sandbox s(limits, 34));
	isolate_sandbox is(limits, 34, 500);
	EXPECT_EQ(is.get_dir(), "/tmp/box/34");
	EXPECT_THROW(isolate_sandbox s(limits, 2365), sandbox_exception);
}

TEST(IsolateSandbox, NormalCommand)
{
	sandbox_limits limits;
	limits.wall_time = 5.1;
	limits.cpu_time = 5.1;
	limits.extra_time = 1;
	limits.disk_blocks = 500;
	limits.disk_inodes = 500;
	limits.memory_usage = 100000;
	limits.std_input = "";
	limits.std_output = "output.txt";
	limits.std_error = "";
	limits.stack_size = 0;
	limits.files_size = 0;
	limits.processes = 0;
	limits.share_net = false;
	isolate_sandbox *is;
	EXPECT_NO_THROW(is = new isolate_sandbox(limits, 34));
	EXPECT_EQ(is->get_dir(), "/tmp/box/34");
	task_results results;
	EXPECT_NO_THROW(results = is->run("/bin/ls", std::vector<std::string>{"-a", "-l", "-i"}));
	EXPECT_TRUE(fs::is_regular_file("/tmp/box/34/box/output.txt"));
	EXPECT_TRUE(fs::file_size("/tmp/box/34/box/output.txt") > 0);
	EXPECT_TRUE(fs::is_regular_file("/tmp/recodex_isolate_34/meta.log"));
	EXPECT_TRUE(fs::file_size("/tmp/recodex_isolate_34/meta.log") > 0);
	EXPECT_TRUE(!results.killed);
	EXPECT_TRUE(results.exitcode == 0);
	EXPECT_TRUE(results.message.empty());
	EXPECT_TRUE(results.wall_time > 0);
	delete is;
}

TEST(IsolateSandbox, TimeoutCommand)
{
	sandbox_limits limits;
	limits.wall_time = 0.5;
	limits.cpu_time = 0.5;
	limits.extra_time = 1;
	limits.disk_blocks = 500;
	limits.disk_inodes = 500;
	limits.memory_usage = 100000;
	limits.std_input = "";
	limits.std_output = "";
	limits.std_error = "";
	limits.stack_size = 0;
	limits.files_size = 0;
	limits.processes = 0;
	limits.share_net = false;
	isolate_sandbox *is;
	EXPECT_NO_THROW(is = new isolate_sandbox(limits, 34));
	EXPECT_EQ(is->get_dir(), "/tmp/box/34");
	task_results results;
	EXPECT_NO_THROW(results = is->run("/bin/sleep", std::vector<std::string>{"5"}));
	EXPECT_TRUE(fs::is_regular_file("/tmp/recodex_isolate_34/meta.log"));
	EXPECT_TRUE(fs::file_size("/tmp/recodex_isolate_34/meta.log") > 0);
	EXPECT_TRUE(results.killed);
	EXPECT_TRUE(results.message == "Time limit exceeded (wall clock)");
	EXPECT_TRUE(results.wall_time > 1);
	EXPECT_TRUE(results.status == isolate_status::TO);
	delete is;
}

TEST(IsolateSandbox, NonzeroReturnCommand)
{
	sandbox_limits limits;
	limits.wall_time = 0.5;
	limits.cpu_time = 0.5;
	limits.extra_time = 1;
	limits.disk_blocks = 500;
	limits.disk_inodes = 500;
	limits.memory_usage = 100000;
	limits.std_input = "";
	limits.std_output = "";
	limits.std_error = "";
	limits.stack_size = 0;
	limits.files_size = 0;
	limits.processes = 0;
	limits.share_net = false;
	isolate_sandbox *is;
	EXPECT_NO_THROW(is = new isolate_sandbox(limits, 34));
	EXPECT_EQ(is->get_dir(), "/tmp/box/34");
	task_results results;
	EXPECT_NO_THROW(results = is->run("/bin/false", std::vector<std::string>{}));
	EXPECT_TRUE(fs::is_regular_file("/tmp/recodex_isolate_34/meta.log"));
	EXPECT_TRUE(fs::file_size("/tmp/recodex_isolate_34/meta.log") > 0);
	EXPECT_TRUE(!results.killed);
	EXPECT_TRUE(results.message == "Exited with error status 1");
	EXPECT_TRUE(results.wall_time > 0);
	EXPECT_TRUE(results.status == isolate_status::RE);
	EXPECT_TRUE(results.exitcode == 1);
	delete is;
}

TEST(IsolateSandbox, TimeoutIsolate)
{
	sandbox_limits limits;
	limits.wall_time = 10;
	limits.cpu_time = 10;
	limits.extra_time = 1;
	limits.disk_blocks = 500;
	limits.disk_inodes = 500;
	limits.memory_usage = 100000;
	limits.std_input = "";
	limits.std_output = "";
	limits.std_error = "";
	limits.stack_size = 0;
	limits.files_size = 0;
	limits.processes = 0;
	limits.share_net = false;
	isolate_sandbox *is;
	EXPECT_NO_THROW(is = new isolate_sandbox(limits, 34, 2));
	EXPECT_EQ(is->get_dir(), "/tmp/box/34");
	task_results results;
	EXPECT_THROW(results = is->run("/bin/sleep", std::vector<std::string>{"5"}), sandbox_exception);
	delete is;
}


#endif

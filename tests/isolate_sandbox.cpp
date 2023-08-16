#ifdef TEST_ISOLATE

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "sandbox/isolate_sandbox.h"

namespace fs = std::filesystem;

TEST(IsolateSandbox, BasicCreation)
{
	std::shared_ptr<sandbox_config> config = std::make_shared<sandbox_config>();
	sandbox_limits limits;
	EXPECT_NO_THROW(isolate_sandbox s(config, limits, 34, "/tmp", ""));
	isolate_sandbox is(config, limits, 34, "/tmp", "");
	EXPECT_EQ(is.get_dir(), "/var/local/lib/isolate/34/box");
	EXPECT_THROW(isolate_sandbox s(config, limits, 2365, "/tmp", ""), sandbox_exception);
}

TEST(IsolateSandbox, NormalCommand)
{
	std::shared_ptr<sandbox_config> config = std::make_shared<sandbox_config>();
	config->std_input = "";
	config->std_output = "output.txt";
	config->std_error = "";
	config->chdir = "";
	sandbox_limits limits;
	limits.wall_time = 5.1;
	limits.cpu_time = 5.1;
	limits.extra_time = 1;
	limits.disk_size = 500;
	limits.disk_files = 500;
	limits.memory_usage = 100000;
	limits.stack_size = 0;
	limits.files_size = 0;
	limits.processes = 0;
	limits.share_net = false;
	limits.bound_dirs.clear();
	isolate_sandbox *is = nullptr;
	EXPECT_NO_THROW(is = new isolate_sandbox(config, limits, 34, "/tmp", ""));
	EXPECT_EQ(is->get_dir(), "/var/local/lib/isolate/34/box");
	sandbox_results results;
	EXPECT_NO_THROW(results = is->run("/bin/ls", std::vector<std::string>{"-a", "-l", "-i"}));
	EXPECT_TRUE(fs::is_regular_file("/var/local/lib/isolate/34/box/output.txt"));
	EXPECT_TRUE(fs::file_size("/var/local/lib/isolate/34/box/output.txt") > 0);
	EXPECT_TRUE(fs::is_regular_file("/tmp/34/meta.log"));
	EXPECT_TRUE(fs::file_size("/tmp/34/meta.log") > 0);
	EXPECT_TRUE(!results.killed);
	EXPECT_TRUE(results.exitcode == 0);
	EXPECT_TRUE(results.message.empty());
	EXPECT_TRUE(results.wall_time > 0);
	delete is;
}

TEST(IsolateSandbox, TimeoutCommand)
{
	std::shared_ptr<sandbox_config> config = std::make_shared<sandbox_config>();
	config->std_input = "";
	config->std_output = "";
	config->std_error = "";
	config->chdir = "";
	sandbox_limits limits;
	limits.wall_time = 0.5;
	limits.cpu_time = 0.5;
	limits.extra_time = 1;
	limits.disk_size = 500;
	limits.disk_files = 500;
	limits.memory_usage = 100000;
	limits.stack_size = 0;
	limits.files_size = 0;
	limits.processes = 0;
	limits.share_net = false;
	limits.bound_dirs.clear();
	isolate_sandbox *is = nullptr;
	EXPECT_NO_THROW(is = new isolate_sandbox(config, limits, 34, "/tmp", ""));
	EXPECT_EQ(is->get_dir(), "/var/local/lib/isolate/34/box");
	sandbox_results results;
	EXPECT_NO_THROW(results = is->run("/bin/sleep", std::vector<std::string>{"5"}));
	EXPECT_TRUE(fs::is_regular_file("/tmp/34/meta.log"));
	EXPECT_TRUE(fs::file_size("/tmp/34/meta.log") > 0);
	EXPECT_TRUE(results.killed);
	EXPECT_TRUE(results.message == "Time limit exceeded (wall clock)");
	EXPECT_TRUE(results.wall_time >= 0.5);
	EXPECT_TRUE(results.status == isolate_status::TO);
	delete is;
}

TEST(IsolateSandbox, NonzeroReturnCommand)
{
	std::shared_ptr<sandbox_config> config = std::make_shared<sandbox_config>();
	config->std_input = "";
	config->std_output = "";
	config->std_error = "";
	config->chdir = "";
	sandbox_limits limits;
	limits.wall_time = 0.5;
	limits.cpu_time = 0.5;
	limits.extra_time = 1;
	limits.disk_size = 500;
	limits.disk_files = 500;
	limits.memory_usage = 100000;
	limits.stack_size = 0;
	limits.files_size = 0;
	limits.processes = 0;
	limits.share_net = false;
	limits.bound_dirs.clear();
	isolate_sandbox *is = nullptr;
	EXPECT_NO_THROW(is = new isolate_sandbox(config, limits, 34, "/tmp", ""));
	EXPECT_EQ(is->get_dir(), "/var/local/lib/isolate/34/box");
	sandbox_results results;
	EXPECT_NO_THROW(results = is->run("/bin/false", std::vector<std::string>{}));
	EXPECT_TRUE(fs::is_regular_file("/tmp/34/meta.log"));
	EXPECT_TRUE(fs::file_size("/tmp/34/meta.log") > 0);
	EXPECT_TRUE(!results.killed);
	EXPECT_TRUE(results.message == "Exited with error status 1");
	EXPECT_TRUE(results.wall_time > 0);
	EXPECT_TRUE(results.status == isolate_status::RE);
	EXPECT_TRUE(results.exitcode == 1);
	delete is;
}

/*TEST(IsolateSandbox, TimeoutIsolate)
{
	sandbox_limits limits;
	limits.wall_time = 10;
	limits.cpu_time = 10;
	limits.extra_time = 1;
	limits.disk_size = 500;
	limits.disk_files = 500;
	limits.memory_usage = 100000;
	limits.std_input = "";
	limits.std_output = "";
	limits.std_error = "";
	limits.stack_size = 0;
	limits.files_size = 0;
	limits.processes = 0;
	limits.share_net = false;
	isolate_sandbox *is = nullptr;
	EXPECT_NO_THROW(is = new isolate_sandbox(limits, 34, "/tmp"));
	EXPECT_EQ(is->get_dir(), "/var/local/lib/isolate/34");
	sandbox_results results;
	EXPECT_THROW(results = is->run("/bin/sleep", std::vector<std::string>{"400"}), sandbox_exception);
	delete is;
}*/

TEST(IsolateSandbox, BindDirsExecuteGCC)
{
	auto tmp = fs::temp_directory_path();
	std::shared_ptr<sandbox_config> config = std::make_shared<sandbox_config>();
	config->chdir = "";
	sandbox_limits limits;
	limits.wall_time = 10;
	limits.cpu_time = 10;
	limits.extra_time = 1;
	limits.processes = 0;
	limits.bound_dirs = {};
	limits.environ_vars = {{"PATH", "/usr/bin"}};

	fs::create_directories(tmp / "recodex_35_test");
	fs::permissions(tmp / "recodex_35_test", fs::perms::group_write | fs::perms::others_write, fs::perm_options::add);
	{
		std::ofstream file((tmp / "recodex_35_test" / "main.c").string());
		file << "#include <stdio.h>\n\nint main(void)\n{\n\tprintf(\"Hello world!\\n\");\n\treturn 0;\n}\n";
	}

	isolate_sandbox *is = nullptr;
	EXPECT_NO_THROW(is = new isolate_sandbox(config, limits, 35, tmp.string(), (tmp / "recodex_35_test").string()));
	sandbox_results results;
	EXPECT_NO_THROW(results = is->run("/usr/bin/gcc", std::vector<std::string>{"-Wall", "-o", "test", "main.c"}));

	EXPECT_TRUE(results.status == isolate_status::OK);
	EXPECT_TRUE(fs::is_regular_file(tmp / "recodex_35_test" / "test"));
	EXPECT_TRUE(fs::file_size(tmp / "recodex_35_test" / "test") > 0);
	EXPECT_TRUE(results.wall_time > 0);
	EXPECT_TRUE(results.memory > 0);
	delete is;
	fs::remove_all(tmp / "recodex_35_test");
}


#endif

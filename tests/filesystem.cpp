#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/helpers/filesystem.h"

typedef std::tuple<std::string, std::string, sandbox_limits::dir_perm> bound_dirs_tuple;
typedef std::vector<bound_dirs_tuple> bound_dirs_type;


TEST(filesystem_test, normalize)
{
	ASSERT_EQ(fs::path().string(), helpers::normalize_path(fs::path(".")).string());
	ASSERT_EQ(fs::path("/").string(), helpers::normalize_path(fs::path("/")).string());
	ASSERT_EQ((fs::path("/first") / fs::path("second") / fs::path("third")).string(),
		helpers::normalize_path(fs::path("/first/second/third")).string());
	ASSERT_EQ((fs::path("first") / fs::path("second") / fs::path("third")).string(),
		helpers::normalize_path(fs::path("first/second/third")).string());
	ASSERT_EQ((fs::path("/second") / fs::path("third")).string(),
		helpers::normalize_path(fs::path("/first/../second/third")).string());
	ASSERT_EQ(fs::path().string(), helpers::normalize_path(fs::path("../hello/hello/hello")).string());
	ASSERT_EQ(fs::path("/third").string(), helpers::normalize_path(fs::path("/first/second/../../third")).string());
	ASSERT_EQ(fs::path().string(), helpers::normalize_path(fs::path("/first/second/../../../third")).string());
	ASSERT_EQ(fs::path().string(), helpers::normalize_path(fs::path("first/second/../../../third")).string());
	ASSERT_EQ(fs::path("/").string(), helpers::normalize_path(fs::path("/././.")).string());
	ASSERT_EQ((fs::path("/first") / fs::path("second") / fs::path("third")).string(),
		helpers::normalize_path(fs::path("/first/./second/third")).string());
	ASSERT_EQ((fs::path("/first") / fs::path("second") / fs::path("third")).string(),
		helpers::normalize_path(fs::path("/first//second/third")).string());
	ASSERT_EQ((fs::path("/first") / fs::path("third")).string(),
		helpers::normalize_path(fs::path("/first////second/../third")).string());
	ASSERT_EQ((fs::path("/first") / fs::path("third")).string(),
		helpers::normalize_path(fs::path("/first////second/../third/././.")).string());
}

TEST(filesystem_test, test_check_relative)
{
	ASSERT_TRUE(helpers::check_relative(fs::path("")));
	ASSERT_TRUE(helpers::check_relative(fs::path(".")));
	ASSERT_TRUE(helpers::check_relative(fs::path("sth")));
	ASSERT_TRUE(helpers::check_relative(fs::path("sth/sth")));
	ASSERT_TRUE(helpers::check_relative(fs::path("./sth")));
	ASSERT_TRUE(helpers::check_relative(fs::path("./sth/sth")));
	//ASSERT_FALSE(helpers::check_relative(fs::path("/"))); // not working on Windows
	//ASSERT_FALSE(helpers::check_relative(fs::path("/sth"))); // not working on Windows
	//ASSERT_FALSE(helpers::check_relative(fs::path("/sth/sth"))); // not working on Windows
	ASSERT_FALSE(helpers::check_relative(fs::path("..")));
	ASSERT_FALSE(helpers::check_relative(fs::path("../sth")));
	ASSERT_FALSE(helpers::check_relative(fs::path("sth/..")));
	ASSERT_FALSE(helpers::check_relative(fs::path("sth/sth/..")));
	ASSERT_FALSE(helpers::check_relative(fs::path("sth/sth/../sth")));
}


bound_dirs_type get_default_dirs()
{
	bound_dirs_type dirs;
	dirs.push_back(bound_dirs_tuple("/path/outside/sandbox", "/inside", sandbox_limits::dir_perm::RW));
	dirs.push_back(bound_dirs_tuple("/another/path/outside/sandbox", "/execute/dir", sandbox_limits::dir_perm::RW));
	return dirs;
}

TEST(filesystem_test, test_relative_1)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("inside", "chdir", dirs, "");
	ASSERT_EQ(fs::path().string(), result.string());
}

TEST(filesystem_test, test_relative_2)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("inside", "/inside", dirs, "");
	ASSERT_EQ((fs::path("/path/outside/sandbox") / fs::path("inside")).string(), result.string());
}

TEST(filesystem_test, test_relative_3)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("inside/file", "/inside", dirs, "");
	ASSERT_EQ((fs::path("/path/outside/sandbox") / fs::path("inside") / fs::path("file")).string(), result.string());
}

TEST(filesystem_test, test_absolute_1)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/output.stderr", "/inside/test1", dirs, "");
	ASSERT_EQ(fs::path().string(), result.string());
}

TEST(filesystem_test, test_absolute_2)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/inside/output.stderr", "/inside/test1", dirs, "");
	ASSERT_EQ((fs::path("/path/outside/sandbox") / fs::path("output.stderr")).string(), result.string());
}

TEST(filesystem_test, test_absolute_3)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/inside/test1/output.stderr", "/inside/test1", dirs, "");
	ASSERT_EQ(
		(fs::path("/path/outside/sandbox") / fs::path("test1") / fs::path("output.stderr")).string(), result.string());
}

TEST(filesystem_test, test_absolute_4)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/inside/test1/sub/output.stderr", "/inside/test1", dirs, "");
	ASSERT_EQ(
		(fs::path("/path/outside/sandbox") / fs::path("test1") / fs::path("sub") / fs::path("output.stderr")).string(),
		result.string());
}

TEST(filesystem_test, test_absolute_5)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/execute/dir/sub/output.stderr", "/inside/test1", dirs, "");
	ASSERT_EQ((fs::path("/another/path/outside/sandbox") / fs::path("sub") / fs::path("output.stderr")).string(),
		result.string());
}

TEST(filesystem_test, test_relative_6)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("../indir", "/inside", dirs, "");
	ASSERT_EQ(fs::path().string(), result.string());
}

TEST(filesystem_test, test_absolute_7)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("../output.stderr", "/inside/test1", dirs, "");
	ASSERT_EQ((fs::path("/path/outside/sandbox") / fs::path("output.stderr")).string(), result.string());
}

TEST(filesystem_test, test_absolute_8)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result =
		helpers::find_path_outside_sandbox("/inside/test1////sub/./output.stderr", "/inside/test1", dirs, "");
	ASSERT_EQ(
		(fs::path("/path/outside/sandbox") / fs::path("test1") / fs::path("sub") / fs::path("output.stderr")).string(),
		result.string());
}

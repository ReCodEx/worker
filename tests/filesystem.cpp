#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/helpers/filesystem.h"

typedef std::tuple<std::string, std::string, sandbox_limits::dir_perm> bound_dirs_tuple;
typedef std::vector<bound_dirs_tuple> bound_dirs_type;


bound_dirs_type get_default_dirs()
{
	bound_dirs_type dirs;
	dirs.push_back(bound_dirs_tuple("/path/outside/sandbox", "/evaluate", sandbox_limits::dir_perm::RW));
	dirs.push_back(bound_dirs_tuple("/another/path/outside/sandbox", "/execute/dir", sandbox_limits::dir_perm::RW));
	return dirs;
}

TEST(filesystem_test, test_relative_1)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("inside", "chdir", dirs);
	ASSERT_EQ("", result.string());
}

TEST(filesystem_test, test_relative_2)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("inside", "/evaluate", dirs);
	ASSERT_EQ("/path/outside/sandbox/inside", result.string());
}

TEST(filesystem_test, test_relative_3)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("inside/file", "/evaluate", dirs);
	ASSERT_EQ("/path/outside/sandbox/inside/file", result.string());
}

TEST(filesystem_test, test_absolute_1)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/output.stderr", "/evaluate/test1", dirs);
	ASSERT_EQ("", result.string());
}

TEST(filesystem_test, test_absolute_2)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/evaluate/output.stderr", "/evaluate/test1", dirs);
	ASSERT_EQ("/path/outside/sandbox/output.stderr", result.string());
}

TEST(filesystem_test, test_absolute_3)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/evaluate/test1/output.stderr", "/evaluate/test1", dirs);
	ASSERT_EQ("/path/outside/sandbox/test1/output.stderr", result.string());
}

TEST(filesystem_test, test_absolute_4)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/evaluate/test1/sub/output.stderr", "/evaluate/test1", dirs);
	ASSERT_EQ("/path/outside/sandbox/test1/sub/output.stderr", result.string());
}

TEST(filesystem_test, test_absolute_5)
{
	bound_dirs_type dirs = get_default_dirs();
	fs::path result = helpers::find_path_outside_sandbox("/execute/dir/sub/output.stderr", "/evaluate/test1", dirs);
	ASSERT_EQ("/another/path/outside/sandbox/sub/output.stderr", result.string());
}

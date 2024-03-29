#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>

#include "fileman/cache_manager.h"

using namespace testing;
using namespace std;

TEST(CacheManager, CacheDir)
{
	auto tmp = fs::temp_directory_path();

	// disabled due to problems on windows machines
	// EXPECT_THROW(cache_manager m("/a"), fm_exception);
	// EXPECT_THROW(cache_manager m("/a/"), fm_exception);
	// EXPECT_THROW(cache_manager o(""), fm_exception);

	EXPECT_NO_THROW(cache_manager p(tmp.string()));
	EXPECT_NO_THROW(cache_manager q("/tmp/"));
	EXPECT_NO_THROW(cache_manager r((tmp / "recodex" / "subdir").string()));
	EXPECT_TRUE(fs::is_directory(tmp / "recodex" / "subdir"));
	fs::remove_all(tmp / "recodex");
}

TEST(CacheManager, GetExistingFile)
{
	auto tmp = fs::temp_directory_path();
	// std::string cache_path_name = "/tmp/recodex/";
	// fs::path cache_path(cache_path_name);
	fs::create_directory(tmp / "recodex");
	{
		ofstream file((tmp / "recodex" / "test.txt").string());
		file << "testing input" << endl;
	}
	cache_manager m((tmp / "recodex").string());
	// Copy file to existing directory
	m.get_file("test.txt", (tmp / "test.txt").string());
	EXPECT_TRUE(fs::is_regular_file((tmp / "test.txt").string()));

	// Copy the same file to same directory - should be overriden
	EXPECT_NO_THROW(m.get_file("test.txt", (tmp / "test.txt").string()));

	// Copy file to nonexisting directory
	EXPECT_THROW(m.get_file("test.txt", (tmp / "recodex" / "nonexist" / "test.txt").string()), fm_exception);

	// Clean used resources
	fs::remove(tmp / "test.txt");
	fs::remove_all(tmp / "recodex");
}

TEST(CacheManager, GetNonexistingFile)
{
	auto tmp = fs::temp_directory_path();
	cache_manager m(tmp.string());
	EXPECT_THROW(m.get_file("ab5cd.txt", "~"), fm_exception);
	EXPECT_THROW(m.get_file((tmp / "as4df.txt").string(), "~"), fm_exception);
	EXPECT_THROW(m.get_file("ab5cd.txt", "/a"), fm_exception);
}

TEST(CacheManager, PutExistingFile)
{
	auto tmp = fs::temp_directory_path();
	{
		ofstream file((tmp / "test.txt").string());
		file << "testing input" << endl;
	}
	cache_manager m((tmp / "recodex").string());
	EXPECT_NO_THROW(m.put_file((tmp / "test.txt").string(), "test.txt"));
	EXPECT_TRUE(fs::is_regular_file((tmp / "recodex" / "test.txt").string()));
	fs::remove((tmp / "test.txt").string());
	fs::remove_all((tmp / "recodex").string());
}

TEST(CacheManager, PutNonexistingFile)
{
	auto tmp = fs::temp_directory_path();
	cache_manager m((tmp / "recodex").string());
	EXPECT_THROW(m.put_file((tmp / "as4df.txt").string(), "as4df.txt"), fm_exception);
	fs::remove_all((tmp / "recodex").string());
}

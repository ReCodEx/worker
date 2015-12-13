#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <fstream>

#include "../src/fileman/cache_manager.h"

using namespace testing;
using namespace std;

TEST(CacheManager, CacheDir)
{
	EXPECT_THROW(cache_manager m("/a"), fm_create_directory_error);
	EXPECT_THROW(cache_manager m("/a/"), fm_create_directory_error);
	EXPECT_THROW(cache_manager o(""), fm_create_directory_error);

	EXPECT_NO_THROW(cache_manager p("/tmp"));
	EXPECT_NO_THROW(cache_manager q("/tmp/"));
	EXPECT_NO_THROW(cache_manager r("/tmp/recodex/dir"));
	fs::remove_all("/tmp/recodex");
}

TEST(CacheManager, GetExistingFile)
{
	std::string cache_path_name = "/tmp/recodex/";
	fs::path cache_path(cache_path_name);
	fs::create_directory(cache_path);
	{
		ofstream file(cache_path_name + "test.txt");
		file << "testing input" << endl;
	}
	cache_manager m(cache_path_name);
	//Copy file to existing directory
	m.get_file("test.txt", "/tmp");
	EXPECT_TRUE(fs::is_regular_file("/tmp/test.txt"));

	//Copy the same file to same directory - should be overriden
	EXPECT_NO_THROW(m.get_file("test.txt", "/tmp"));

	//Copy file to nonexisting directory
	EXPECT_THROW(m.get_file("test.txt", "/a"), fm_copy_error);

	//Clean used resources
	fs::remove("/tmp/test.txt");
	fs::remove_all(cache_path);
}

TEST(CacheManager, GetNonexistingFile)
{
	cache_manager m("/tmp");
	EXPECT_THROW(m.get_file("abcd.txt", "~"), fm_copy_error);
	EXPECT_THROW(m.get_file("/tmp/abcd.txt", "~"), fm_copy_error);
	EXPECT_THROW(m.get_file("abcd.txt", "/a"), fm_copy_error);
}

TEST(CacheManager, PutExistingFile)
{
	{
		ofstream file("/tmp/test.txt");
		file << "testing input" << endl;
	}
	cache_manager m("/tmp/recodex");
	EXPECT_NO_THROW(m.put_file("/tmp/test.txt"));
	EXPECT_TRUE(fs::is_regular_file("/tmp/recodex/test.txt"));
	fs::remove("/tmp/test.txt");
	fs::remove_all("/tmp/recidex");
}

TEST(CacheManager, PutNonexistingFile)
{
	cache_manager m("/tmp/recodex");
	EXPECT_THROW(m.put_file("/tmp/asdfg.txt"), fm_copy_error);
	fs::remove_all("/tmp/recodex");
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <string>
#include <memory>
#include <utility>

#include "../src/fileman/file_manager_base.h"
#include "../src/fileman/http_manager.h"
#include "../src/fileman/cache_manager.h"
#include "../src/fileman/file_manager.h"

using namespace testing;
using namespace std;


class mock_cache_manager : public cache_manager {
public:
	mock_cache_manager() {}
	MOCK_CONST_METHOD0(get_caching_dir, std::string());
	MOCK_METHOD2(put_file, void(const std::string &name, const std::string &dst_path));
	MOCK_METHOD2(get_file, void(const std::string &src_name, const std::string &dst_path));
};

class mock_http_manager : public http_manager {
public:
	mock_http_manager() {}
	MOCK_METHOD2(put_file, void(const std::string &name, const std::string &dst_path));
	MOCK_METHOD2(get_file, void(const std::string &src_name, const std::string &dst_path));
	MOCK_METHOD2(set_params, void(const std::string &username, const std::string &password));
};

TEST(FileManager, GetFileFromCache)
{
	auto cache = unique_ptr<mock_cache_manager>(new mock_cache_manager);
	auto remote = unique_ptr<mock_http_manager>(new mock_http_manager);

	EXPECT_CALL((*cache), get_file("file.txt", "/tmp/")).Times(1);

	file_manager m(move(cache), move(remote));
	m.get_file("file.txt", "/tmp/");
}

TEST(FileManager, GetFileFromRemote)
{
	auto cache = unique_ptr<mock_cache_manager>(new mock_cache_manager);
	auto remote = unique_ptr<mock_http_manager>(new mock_http_manager);

	{
		InSequence s;
		EXPECT_CALL((*cache), get_file("file.txt", "/tmp/"))
				.WillOnce(Throw(fm_exception("")));
		EXPECT_CALL((*remote), get_file("file.txt", "/tmp/recodex/"))
				.Times(1);
		EXPECT_CALL((*cache), get_file("file.txt", "/tmp/"))
				.Times(1);
	}

	file_manager m(move(cache), move(remote));
	EXPECT_NO_THROW(m.get_file("file.txt", "/tmp/"));
}

TEST(FileManager, PutFileToRemote)
{
	auto cache = unique_ptr<mock_cache_manager>(new mock_cache_manager);
	auto remote = unique_ptr<mock_http_manager>(new mock_http_manager);

	EXPECT_CALL((*remote), put_file("file.txt")).Times(1);

	file_manager m(move(cache), move(remote));
	EXPECT_NO_THROW(m.put_file("file.txt"));
}

TEST(FileManager, GetOperationFailed)
{
	auto cache = unique_ptr<mock_cache_manager>(new mock_cache_manager);
	auto remote = unique_ptr<mock_http_manager>(new mock_http_manager);

	EXPECT_CALL((*cache), get_file("file.txt", "/tmp/"))
			.WillOnce(Throw(fm_exception("")));
	EXPECT_CALL((*remote), get_file("file.txt", "/tmp/recodex/"))
			.WillOnce(Throw(fm_exception("")));

	file_manager m(move(cache), move(remote));
	EXPECT_THROW(m.get_file("file.txt", "/tmp/"), fm_exception);
}

TEST(FileManager, PutOperationFailed)
{
	auto cache = unique_ptr<mock_cache_manager>(new mock_cache_manager);
	auto remote = unique_ptr<mock_http_manager>(new mock_http_manager);

	EXPECT_CALL((*remote), put_file("file.txt"))
			.WillOnce(Throw(fm_exception("")));

	file_manager m(move(cache), move(remote));
	EXPECT_THROW(m.put_file("file.txt"), fm_exception);
}

TEST(FileManager, SetGetParams)
{
	auto cache = unique_ptr<mock_cache_manager>(new mock_cache_manager);
	auto remote = unique_ptr<mock_http_manager>(new mock_http_manager);

	EXPECT_CALL((*remote), set_params("newURL", "user", "pass"));

	file_manager m(move(cache), move(remote));
	m.set_params("user", "pass");
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <string>
#include <memory>
#include <utility>

#include "../src/fileman/file_manager_base.h"
#include "../src/fileman/fallback_file_manager.h"

using namespace testing;
using namespace std;


class mock_file_manager : public file_manager_base
{
public:
	mock_file_manager()
	{
	}
	MOCK_CONST_METHOD0(get_caching_dir, std::string());
	MOCK_METHOD2(put_file, void(const std::string &name, const std::string &dst_path));
	MOCK_METHOD2(get_file, void(const std::string &src_name, const std::string &dst_path));
};

TEST(fallback_file_manager, GetFileFromCache)
{
	auto cache = unique_ptr<mock_file_manager>(new mock_file_manager);
	auto remote = unique_ptr<mock_file_manager>(new StrictMock<mock_file_manager>);

	std::string remote_path = "file.txt";
	std::string local_path = "/tmp/file.txt";

	EXPECT_CALL((*cache), get_file(remote_path, local_path)).Times(1);

	fallback_file_manager m(move(cache), move(remote));
	m.get_file(remote_path, local_path);
}

TEST(fallback_file_manager, GetFileFromRemote)
{
	auto cache = unique_ptr<mock_file_manager>(new mock_file_manager);
	auto remote = unique_ptr<mock_file_manager>(new mock_file_manager);

	std::string remote_path = "file.txt";
	std::string local_path = "/tmp/file.txt";

	{
		InSequence s;
		EXPECT_CALL((*cache), get_file(remote_path, local_path)).WillOnce(Throw(fm_exception("")));
		EXPECT_CALL((*remote), get_file(remote_path, local_path)).Times(1);
		EXPECT_CALL((*cache), put_file(local_path, remote_path)).Times(1);
	}

	fallback_file_manager m(move(cache), move(remote));
	EXPECT_NO_THROW(m.get_file(remote_path, local_path));
}

TEST(fallback_file_manager, PutFileToRemote)
{
	auto cache = unique_ptr<mock_file_manager>(new mock_file_manager);
	auto remote = unique_ptr<mock_file_manager>(new mock_file_manager);

	EXPECT_CALL((*remote), put_file("file.txt", "file.txt")).Times(1);

	fallback_file_manager m(move(cache), move(remote));
	EXPECT_NO_THROW(m.put_file("file.txt", "file.txt"));
}

TEST(fallback_file_manager, GetOperationFailed)
{
	auto cache = unique_ptr<mock_file_manager>(new mock_file_manager);
	auto remote = unique_ptr<mock_file_manager>(new mock_file_manager);

	std::string remote_path = "file.txt";
	std::string local_path = "/tmp/file.txt";

	EXPECT_CALL((*cache), get_file(remote_path, local_path)).WillOnce(Throw(fm_exception("")));
	EXPECT_CALL((*remote), get_file(remote_path, local_path)).WillOnce(Throw(fm_exception("")));

	fallback_file_manager m(move(cache), move(remote));
	EXPECT_THROW(m.get_file(remote_path, local_path), fm_exception);
}

TEST(fallback_file_manager, PutOperationFailed)
{
	auto cache = unique_ptr<mock_file_manager>(new mock_file_manager);
	auto remote = unique_ptr<mock_file_manager>(new mock_file_manager);

	std::string remote_path = "file.txt";
	std::string local_path = "/tmp/file.txt";

	EXPECT_CALL((*remote), put_file(local_path, remote_path)).WillOnce(Throw(fm_exception("")));

	fallback_file_manager m(move(cache), move(remote));
	EXPECT_THROW(m.put_file(local_path, remote_path), fm_exception);
}

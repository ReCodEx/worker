#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <string>
#include <memory>
#include <utility>

#include "mocks.h"
#include "fileman/fallback_file_manager.h"

using namespace testing;
using namespace std;


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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <filesystem>

#include "archives/archivator.h"

namespace fs = std::filesystem;


TEST(Archivator, DecompressNonexistingArchive)
{
	EXPECT_THROW(archivator::decompress("abcd.zip", "./"), archive_exception);
}

TEST(Archivator, DecompressZip)
{
	fs::path unziped_path = fs::temp_directory_path() / "valid_zip";
	fs::create_directory(unziped_path);

	ASSERT_NO_THROW(archivator::decompress("testing_archives/valid_zip.zip", unziped_path.string()));
	EXPECT_TRUE(fs::is_directory(unziped_path));
	EXPECT_TRUE(fs::is_regular_file(unziped_path / "a.txt"));
	EXPECT_TRUE(fs::file_size(unziped_path / "a.txt") > 0);
	fs::remove_all(unziped_path);
}

TEST(Archivator, DecompressTar)
{
	fs::path untared_path = fs::temp_directory_path() / "valid_tar";
	fs::create_directory(untared_path);

	ASSERT_NO_THROW(archivator::decompress("testing_archives/valid_tar.tar", untared_path.string()));
	EXPECT_TRUE(fs::is_directory(untared_path));
	EXPECT_TRUE(fs::is_regular_file(untared_path / "a.txt"));
	EXPECT_TRUE(fs::file_size(untared_path / "a.txt") > 0);
	fs::remove_all(untared_path);
}

TEST(Archivator, DecompressTarGz)
{
	fs::path untared_path = fs::temp_directory_path() / "valid_tar";
	fs::create_directory(untared_path);

	ASSERT_NO_THROW(archivator::decompress("testing_archives/valid_tar.tar.gz", untared_path.string()));
	EXPECT_TRUE(fs::is_directory(untared_path));
	EXPECT_TRUE(fs::is_regular_file(untared_path / "a.txt"));
	EXPECT_TRUE(fs::file_size(untared_path / "a.txt") > 0);
	fs::remove_all(untared_path);
}

TEST(Archivator, DecompressTarBz2)
{
	fs::path untared_path = fs::temp_directory_path() / "valid_tar";
	fs::create_directory(untared_path);

	ASSERT_NO_THROW(archivator::decompress("testing_archives/valid_tar.tar.bz2", untared_path.string()));
	EXPECT_TRUE(fs::is_directory(untared_path));
	EXPECT_TRUE(fs::is_regular_file(untared_path / "a.txt"));
	EXPECT_TRUE(fs::file_size(untared_path / "a.txt") > 0);
	fs::remove_all(untared_path);
}

TEST(Archivator, DecompressCorruptedZip)
{
	EXPECT_THROW(archivator::decompress("testing_archives/corrupted_zip.zip", fs::temp_directory_path().string()),
		archive_exception);
}

TEST(Archivator, DecompressDotPathZip)
{
	EXPECT_THROW(
		archivator::decompress("testing_archives/dot_path.zip", fs::temp_directory_path().string()), archive_exception);
}

TEST(Archivator, CompressZip)
{
	ASSERT_NO_THROW(archivator::compress(".", (fs::temp_directory_path() / "archive.zip").string()));
	EXPECT_TRUE(fs::is_regular_file(fs::temp_directory_path() / "archive.zip"));
	EXPECT_TRUE(fs::file_size(fs::temp_directory_path() / "archive.zip") > 0);
	fs::remove_all(fs::temp_directory_path() / "archive.zip");
}

TEST(Archivator, CompressNonexistingDir)
{
	EXPECT_THROW(archivator::compress((fs::current_path().root_path() / "nonexisting_dir").string(),
					 (fs::temp_directory_path() / "archive.zip").string()),
		archive_exception);
}

TEST(Archivator, CompressAbsolutePath)
{
	auto archive_path = fs::temp_directory_path() / "archive_test";
	fs::create_directories(archive_path);
	fs::path test_file_path = archive_path / "test_file.txt";
	fs::path result_path = fs::temp_directory_path() / "archive.zip";
	fs::path extracted_path = fs::temp_directory_path() / "archive";

	{
		std::ofstream test_file(test_file_path.string());
		test_file << "1234567";
	}

	archivator::compress(archive_path.string(), result_path.string());
	ASSERT_TRUE(fs::is_regular_file(result_path));

	ASSERT_NO_THROW(archivator::decompress(result_path.string(), fs::temp_directory_path().string()));
	ASSERT_TRUE(fs::is_regular_file(extracted_path / "test_file.txt"));
	ASSERT_EQ((std::size_t) 7, fs::file_size(extracted_path / "test_file.txt"));

	fs::remove_all(archive_path);
	fs::remove_all(extracted_path);
	fs::remove(result_path);
}

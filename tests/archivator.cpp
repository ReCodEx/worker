#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/archives/archivator.h"
\
#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


TEST(Archivator, DecompressNonexistingArchive)
{
	EXPECT_THROW(archivator::decompress("abcd.zip", "./"), archive_exception);
}

TEST(Archivator, DecompressZip)
{
	EXPECT_NO_THROW(archivator::decompress("../tests/testing_archives/valid_zip.zip", fs::temp_directory_path().string()));
	EXPECT_TRUE(fs::is_directory(fs::temp_directory_path() / "valid_zip"));
	EXPECT_TRUE(fs::is_regular_file(fs::temp_directory_path() / "valid_zip" / "a.txt"));
	EXPECT_TRUE(fs::file_size(fs::temp_directory_path() / "valid_zip" / "a.txt") > 0);
	fs::remove_all(fs::temp_directory_path() / "valid_zip");
}

TEST(Archivator, DecompressTar)
{
	EXPECT_NO_THROW(archivator::decompress("../tests/testing_archives/valid_tar.tar", fs::temp_directory_path().string()));
	EXPECT_TRUE(fs::is_directory(fs::temp_directory_path() / "valid_tar"));
	EXPECT_TRUE(fs::is_regular_file(fs::temp_directory_path() / "valid_tar" / "a.txt"));
	EXPECT_TRUE(fs::file_size(fs::temp_directory_path() / "valid_tar" / "a.txt") > 0);
	fs::remove_all(fs::temp_directory_path() / "valid_tar");
}

TEST(Archivator, DecompressTarGz)
{
	EXPECT_NO_THROW(archivator::decompress("../tests/testing_archives/valid_tar.tar.gz", fs::temp_directory_path().string()));
	EXPECT_TRUE(fs::is_directory(fs::temp_directory_path() / "valid_tar"));
	EXPECT_TRUE(fs::is_regular_file(fs::temp_directory_path() / "valid_tar" / "a.txt"));
	EXPECT_TRUE(fs::file_size(fs::temp_directory_path() / "valid_tar" / "a.txt") > 0);
	fs::remove_all(fs::temp_directory_path() / "valid_tar");
}

TEST(Archivator, DecompressTarBz2)
{
	EXPECT_NO_THROW(archivator::decompress("../tests/testing_archives/valid_tar.tar.bz2", fs::temp_directory_path().string()));
	EXPECT_TRUE(fs::is_directory(fs::temp_directory_path() / "valid_tar"));
	EXPECT_TRUE(fs::is_regular_file(fs::temp_directory_path() / "valid_tar" / "a.txt"));
	EXPECT_TRUE(fs::file_size(fs::temp_directory_path() / "valid_tar" / "a.txt") > 0);
	fs::remove_all(fs::temp_directory_path() / "valid_tar");
}

TEST(Archivator, DecompressCorruptedZip)
{
	EXPECT_THROW(archivator::decompress("../tests/testing_archives/corrupted_zip.zip",
										fs::temp_directory_path().string()), archive_exception);
}

TEST(Archivator, DecompressDotPathZip)
{
	EXPECT_THROW(archivator::decompress("../tests/testing_archives/dot_path.zip",
										fs::temp_directory_path().string()), archive_exception);
}

TEST(Archivator, CompressZip)
{
	EXPECT_NO_THROW(archivator::compress("../tests", (fs::temp_directory_path() / "archive.zip").string()));
	EXPECT_TRUE(fs::is_regular_file(fs::temp_directory_path() / "archive.zip"));
	EXPECT_TRUE(fs::file_size(fs::temp_directory_path() / "archive.zip") > 0);
	fs::remove_all(fs::temp_directory_path() / "archive.zip");
}

TEST(Archivator, CompressNonexistingDir)
{
	EXPECT_THROW(archivator::compress((fs::current_path().root_path() / "nonexisting_dir").string(),
										 (fs::temp_directory_path() / "archive.zip").string()), archive_exception);
}

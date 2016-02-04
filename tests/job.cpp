#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "../src/job/job.h"
#include "../src/fileman/cache_manager.h"
#include "../src/job/job_exception.h"


TEST(job_test, bad_parameters)
{
	std::shared_ptr<job_metadata> job_meta = std::make_shared<job_metadata>();
	std::shared_ptr<worker_config> worker_conf = std::make_shared<worker_config>();

	// job_config not given
	EXPECT_THROW(job(nullptr, nullptr, "", nullptr), job_exception);

	// worker_config not given
	EXPECT_THROW(job(job_meta, nullptr, "", nullptr), job_exception);

	// source path not exists
	EXPECT_THROW(job(job_meta, worker_conf, "", nullptr), job_exception);

	// fileman not given
	EXPECT_THROW(job(job_meta, worker_conf, temp_directory_path(), nullptr), job_exception);
}

/*TEST(job_test, bad_paths)
{
	// prepare all things which need to be prepared
	path dir_root = temp_directory_path();
	dir_root = dir_root / "isoeval";
	path dir = dir_root;
	dir = dir / "job_test";
	auto yaml = YAML::Load("");
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);

	// non-existing source code folder
	EXPECT_THROW(job_metadata j(yaml, conf_ptr), job_exception);

	// source code path with no source codes in it
	create_directories(dir);
	EXPECT_THROW(job_metadata j(yaml, conf_ptr), job_exception);

	// source code directory is not a directory
	dir = dir / "hello";
	std::ofstream hello(dir.string());
	hello << "hello" << std::endl;
	hello.close();
	EXPECT_THROW(job_metadata j(yaml, conf_ptr), job_exception);

	// cleanup after yourself
	remove_all(dir_root);
}

TEST(job_test, config_data) // TODO
{
	// prepare all things which need to be prepared
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);
	path dir_root = temp_directory_path() / "isoeval";
	path dir = dir_root / "job_test";
	create_directories(dir);
	std::ofstream hello((dir / "hello").string());
	hello << "hello" << std::endl;
	hello.close();

	// empty configuration
	auto yaml = YAML::Load("");
	EXPECT_THROW(job_metadata j(yaml, conf_ptr), job_exception);

	// cleanup
	remove_all(dir_root);
}*/

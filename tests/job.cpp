#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

#include "../src/tasks/job.h"
#include "../src/fileman/cache_manager.h"

static std::string job_config = "";

TEST(job_test, bad_parameters)
{
	auto yaml = YAML::Load("");

	// config not given
	EXPECT_THROW(job j(yaml, "", nullptr, nullptr, nullptr), job_exception);

	// file manager not given
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	EXPECT_THROW(job j(yaml, "", nullptr, conf_ptr, nullptr), job_exception);

	// source code path not given
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);
	EXPECT_THROW(job j(yaml, "", nullptr, conf_ptr, fileman_ptr), job_exception);
}

TEST(job_test, bad_paths)
{
	using namespace boost::filesystem;

	// prepare all things which need to be prepared
	path dir = temp_directory_path();
	dir = dir / "isoeval" / "job_test";
	auto yaml = YAML::Load(job_config);
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);

	// non-existing source code folder
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// source code path with no source codes in it
	create_directories(dir);
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);

	// source code directory is not a directory
	dir = dir / "hello";
	std::ofstream hello(dir.string());
	hello << "hello" << std::endl;
	hello.close();
	EXPECT_THROW(job j(yaml, dir.string(), nullptr, conf_ptr, fileman_ptr), job_exception);
}

TEST(job_test, bad_config_format)
{
	// prepare all things which need to be prepared
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);

	//EXPECT_THROW(job j("", nullptr, conf_ptr, fileman_ptr), job_exception);
}

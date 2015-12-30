#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

#include "../src/tasks/job.h"
#include "../src/fileman/cache_manager.h"

TEST(job_test, bad_parameters)
{
	// config not given
	EXPECT_THROW(job j("", nullptr, nullptr, nullptr), job_exception);

	// file manager not given
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	EXPECT_THROW(job j("", nullptr, conf_ptr, nullptr), job_exception);

	// submission path not given
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);
	EXPECT_THROW(job j("", nullptr, conf_ptr, fileman_ptr), job_exception);
}

TEST(job_test, bad_paths)
{
	// prepare all things which need to be prepared
	worker_config conf;
	auto conf_ptr = std::make_shared<worker_config>(conf);
	cache_manager fileman;
	auto fileman_ptr = std::make_shared<cache_manager>(fileman);

	// non-existing folder
	EXPECT_THROW(job j("/tmp/isoeval/job_test", nullptr, conf_ptr, fileman_ptr), job_exception);

	// submission path with config and not with source codes

	// missing configuration in submission path

	// not readable configuration file
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

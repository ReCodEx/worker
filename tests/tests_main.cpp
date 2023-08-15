#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include <curl/curl.h>
#include <filesystem>

namespace fs = std::filesystem;


/**
 * Function for global initialization of used resources.
 */
void init()
{
	// Globally init curl library
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

/**
 * Function for global destruction of initialized resources.
 */
void fini()
{
	// Clean after curl library
	curl_global_cleanup();
}

int main(int argc, char **argv)
{
	try {
		init();
	} catch (...) {
		return 1;
	}
	testing::InitGoogleTest(&argc, argv);
	auto result = RUN_ALL_TESTS();
	fini();
	return result;
}

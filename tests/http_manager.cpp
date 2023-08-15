#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>

#include "fileman/http_manager.h"

using namespace testing;
using namespace std;
namespace fs = std::filesystem;


static fileman_config get_recodex_config()
{
	fileman_config conf;
	conf.remote_url = "https://recodex.mff.cuni.cz:8082";
	conf.username = "re";
	conf.password = "codex";
	return conf;
}


// Disabled: server no longer exist
TEST(HttpManager, DISABLED_GetExistingFile)
{
	auto tmp = fs::temp_directory_path();
	fileman_config config = get_recodex_config();
	http_manager m({config});
	EXPECT_NO_THROW(m.get_file(config.remote_url + "/fm_test/test1.txt", (tmp / "test1.txt").string()));
	EXPECT_TRUE(fs::is_regular_file((tmp / "test1.txt").string()));
	fs::remove(tmp / "test1.txt");
}

TEST(HttpManager, DISABLED_GetExistingFileRedirect)
{
	auto tmp = fs::temp_directory_path();
	fileman_config config = get_recodex_config();
	http_manager m({config});
	EXPECT_NO_THROW(m.get_file(config.remote_url + "/fm_test/test2.txt", (tmp / "test2.txt").string()));
	EXPECT_TRUE(fs::is_regular_file((tmp / "test2.txt").string()));
	fs::remove(tmp / "test2.txt");
}

TEST(HttpManager, GetExistingHttp)
{
	auto tmp = fs::temp_directory_path();
	fileman_config config;
	config.remote_url = "https://curl.se";
	config.username = "";
	config.password = "";
	http_manager m({config});
	EXPECT_NO_THROW(m.get_file("https://curl.se/rfc/rfc7234.txt", (tmp / "rfc7234.txt").string()));
	EXPECT_TRUE(fs::is_regular_file((tmp / "rfc7234.txt").string()));
	fs::remove(tmp / "rfc7234.txt");
}

// Disabled: server no longer exist
TEST(HttpManager, DISABLED_GetNonexistingFile)
{
	auto tmp = fs::temp_directory_path();
	fileman_config config = get_recodex_config();
	http_manager m({config});
	EXPECT_THROW(
		m.get_file(config.remote_url + "/fm_test_nonexist/test3.txt", (tmp / "test3.txt").string()), fm_exception);
	EXPECT_FALSE(fs::is_regular_file((tmp / "test3.txt").string()));
	//*
	try {
		fs::remove(tmp / "test3.txt");
	} catch (...) {
	}
	//*/
}

// Disabled: server no longer exist
TEST(HttpManager, DISABLED_WrongAuthentication)
{
	auto tmp = fs::temp_directory_path();
	fileman_config config = get_recodex_config();
	config.username = "codex";
	config.password = "re";
	http_manager m({config});
	EXPECT_THROW(m.get_file(config.remote_url + "/fm_test/test1.txt", (tmp / "test1.txt").string()), fm_exception);
	EXPECT_FALSE(fs::is_regular_file((tmp / "test1.txt").string()));
	fs::remove(tmp / "test1.txt");
}

TEST(HttpManager, NonexistingServer)
{
	auto tmp = fs::temp_directory_path();
	fileman_config config;
	config.remote_url = "http://abcd.example.com";
	config.username = "a";
	config.password = "a";
	http_manager m({config});
	EXPECT_THROW(m.get_file("http://abcd.example.com/test1.txt", (tmp / "test1.txt").string()), fm_exception);
	EXPECT_FALSE(fs::is_regular_file((tmp / "test1.txt").string()));
	fs::remove(tmp / "test1.txt");
}

// Not testing now ...
/*TEST(HttpManager, ValidInvalidURLs)
{
	EXPECT_NO_THROW(http_manager m("https://ps.stdin.cz:8443/", "", ""));
	EXPECT_NO_THROW(http_manager m("http://ps.stdin.cz/", "", ""));

	EXPECT_THROW(http_manager m("htt://a.b/", "", ""), fm_exception);
	EXPECT_THROW(http_manager m("http://a.b", "", ""), fm_exception);
	EXPECT_THROW(http_manager m("htt://a/", "", ""), fm_exception);
	EXPECT_THROW(http_manager m("htt://a.b/", "", ""), fm_exception);
}*/


std::vector<std::string> codes{
	"400", // Bad Request
	"401", // Unauthorized
	"402", // Payment Required
	"403", // Forbidden
	"404", // Not Found
	"405", // Method Not Allowed
	"406", // Not Acceptable
	"407", // Proxy Authentication Required
	"408", // Request Timeout
	"409", // Conflict
	"410", // Gone
	"411", // Length Required
	"412", // Precondition Required
	"413", // Request Entry Too Large
	"414", // Request-URI Too Long
	"415", // Unsupported Media Type
	"416", // Requested Range Not Satisfiable
	"417", // Expectation Failed
	"418", // I'm a teapot
	"422", // Unprocessable Entity
	"428", // Precondition Required
	"429", // Too Many Requests
	"431", // Request Header Fields Too Large
	"500", // Internal Server Error
	"501", // Not Implemented
	"502", // Bad Gateway
	"503", // Service Unavailable
	"504", // Gateway Timeout
	"505", // HTTP Version Not Supported
	"511", // Network Authentication Required
	"520", // Web server is returning an unknown error
	"522", // Connection timed out
	"524" // A timeout occurred
};

// Disabled: server no longer exist
TEST(HttpManager, DISABLED_AllWrongErrorCodes)
{
	auto tmp = fs::temp_directory_path();
	fileman_config config;
	config.remote_url = "http://httpstat.us";
	config.username = "";
	config.password = "";
	http_manager m({config});
	for (auto &i : codes) {
		// std::cout << i << std::endl;
		EXPECT_THROW(m.get_file(std::string("http://httpstat.us/") + i, (tmp / "test.txt").string()), fm_exception);
		EXPECT_FALSE(fs::is_regular_file(tmp / i));
	}
}

// Disabled: server no longer exist
TEST(HttpManager, DISABLED_SimplePutFile)
{
	auto tmp = fs::temp_directory_path();
	{
		ofstream o((tmp / "a.txt").string());
		o << "testing string" << endl;
	}

	fileman_config config1 = get_recodex_config();

	http_manager m1({config1});
	EXPECT_NO_THROW(m1.put_file((tmp / "a.txt").string(), config1.remote_url + "/fm_test/a.txt"));

	// Invalid credentials
	fileman_config config2 = get_recodex_config();
	config2.username = "codex";
	config2.password = "re";
	http_manager m2({config2});
	EXPECT_THROW(m2.put_file((tmp / "a.txt").string(), config2.remote_url + "/fm_test/a.txt"), fm_exception);

	fs::remove(tmp / "a.txt");
}

// Disabled: server no longer exist
TEST(HttpManager, DISABLED_PutFileWrongURL)
{
	auto tmp = fs::temp_directory_path();
	{
		ofstream o((tmp / "b.txt").string());
		o << "testing string" << endl;
	}

	fileman_config config = get_recodex_config();
	http_manager m({config});
	EXPECT_THROW(m.put_file((tmp / "b.txt").string(), config.remote_url + "/fm_test_nonexist/b.txt"), fm_exception);

	EXPECT_THROW(m.put_file((tmp / "b.txt").string(), "http://ps.stdin.cz/b.txt"), fm_exception);

	fs::remove(tmp / "b.txt");
}

// Disabled: server no longer exist
TEST(HttpManager, DISABLED_PutWrongFile)
{
	auto tmp = fs::temp_directory_path();
	fileman_config config = get_recodex_config();
	http_manager m({config});
	EXPECT_THROW(m.put_file((tmp / "abc5xyz.txt").string(), config.remote_url + "/fm_test/abc5xyz.txt"), fm_exception);
}

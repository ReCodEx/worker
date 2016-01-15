#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <vector>
#include <iostream>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

#include "../src/fileman/http_manager.h"

using namespace testing;
using namespace std;
namespace fs = boost::filesystem;


TEST(HttpManager, GetExistingFile)
{
	auto tmp = fs::temp_directory_path();
	http_manager m("re", "codex");
	EXPECT_NO_THROW(m.get_file("https://recodex.projekty.ms.mff.cuni.cz/fm_test/test1.txt", (tmp / "test1.txt").string()));
	EXPECT_TRUE(fs::is_regular_file((tmp / "test1.txt").string()));
	fs::remove(tmp / "test1.txt");
}

TEST(HttpManager, GetExistingFileRedirect)
{
	auto tmp = fs::temp_directory_path();
	http_manager m("re", "codex");
	EXPECT_NO_THROW(m.get_file("https://recodex.projekty.ms.mff.cuni.cz/fm_test/test2.txt", (tmp / "test2.txt").string()));
	EXPECT_TRUE(fs::is_regular_file((tmp / "test2.txt").string()));
	fs::remove(tmp / "test2.txt");
}

TEST(HttpManager, GetExistingHttp)
{
	auto tmp = fs::temp_directory_path();
	http_manager m("", "");
	EXPECT_NO_THROW(m.get_file("http://curl.haxx.se/rfc/rfc7234.txt", (tmp / "rfc7234.txt").string()));
	EXPECT_TRUE(fs::is_regular_file((tmp / "rfc7234.txt").string()));
	fs::remove(tmp / "rfc7234.txt");
}

TEST(HttpManager, GetNonexistingFile)
{
	auto tmp = fs::temp_directory_path();
	http_manager m("re", "codex");
	EXPECT_THROW(m.get_file("https://recodex.projekty.ms.mff.cuni.cz/fm_test_nonexist/test3.txt", (tmp / "test3.txt").string()), fm_exception);
	EXPECT_FALSE(fs::is_regular_file((tmp / "test3.txt").string()));
	//*
	try {
		fs::remove(tmp / "test3.txt");
	} catch(...) {}
	//*/
}

TEST(HttpManager, WrongAuthentication)
{
	auto tmp = fs::temp_directory_path();
	http_manager m("codex", "re");
	EXPECT_THROW(m.get_file("https://recodex.projekty.ms.mff.cuni.cz/fm_test/test1.txt", (tmp / "test1.txt").string()), fm_exception);
	EXPECT_FALSE(fs::is_regular_file((tmp / "test1.txt").string()));
	fs::remove(tmp / "test1.txt");
}

TEST(HttpManager, NonexistingServer)
{
	auto tmp = fs::temp_directory_path();
	http_manager m("a", "a");
	EXPECT_THROW(m.get_file("http://abcd.example.com/test1.txt", (tmp / "test1.txt").string()), fm_exception);
	EXPECT_FALSE(fs::is_regular_file((tmp / "test1.txt").string()));
	fs::remove(tmp / "test1.txt");
}

//Not testing now ...
/*TEST(HttpManager, ValidInvalidURLs)
{
	EXPECT_NO_THROW(http_manager m("https://ps.stdin.cz:8443/", "", ""));
	EXPECT_NO_THROW(http_manager m("http://ps.stdin.cz/", "", ""));

	EXPECT_THROW(http_manager m("htt://a.b/", "", ""), fm_exception);
	EXPECT_THROW(http_manager m("http://a.b", "", ""), fm_exception);
	EXPECT_THROW(http_manager m("htt://a/", "", ""), fm_exception);
	EXPECT_THROW(http_manager m("htt://a.b/", "", ""), fm_exception);
}*/


std::vector<std::string> codes {
		"400",	//Bad Request
		"401",	//Unauthorized
		"402",	//Payment Required
		"403",	//Forbidden
		"404",	//Not Found
		"405",	//Method Not Allowed
		"406",	//Not Acceptable
		"407",	//Proxy Authentication Required
		"408",	//Request Timeout
		"409",	//Conflict
		"410",	//Gone
		"411",	//Length Required
		"412",	//Precondition Required
		"413",	//Request Entry Too Large
		"414",	//Request-URI Too Long
		"415",	//Unsupported Media Type
		"416",	//Requested Range Not Satisfiable
		"417",	//Expectation Failed
		"418",	//I'm a teapot
		"422",	//Unprocessable Entity
		"428",	//Precondition Required
		"429",	//Too Many Requests
		"431",	//Request Header Fields Too Large
		"500",	//Internal Server Error
		"501",	//Not Implemented
		"502",	//Bad Gateway
		"503",	//Service Unavailable
		"504",	//Gateway Timeout
		"505",	//HTTP Version Not Supported
		"511",	//Network Authentication Required
		"520",	//Web server is returning an unknown error
		"522",	//Connection timed out
		"524"	//A timeout occurred
};
TEST(HttpManager, DISABLED_AllWrongErrorCodes) {
	auto tmp = fs::temp_directory_path();
	http_manager m("", "");
	for(auto& i : codes) {
		//std::cout << i << std::endl;
		EXPECT_THROW(m.get_file(std::string("http://httpstat.us/") + i, (tmp / "test.txt").string()), fm_exception);
		EXPECT_FALSE(fs::is_regular_file(tmp / i));
	}
}

TEST(HttpManager, SimplePutFile)
{
	auto tmp = fs::temp_directory_path();
	{
		ofstream o((tmp / "a.txt").string());
		o << "testing string" << endl;
	}

	http_manager m("re", "codex");
	EXPECT_NO_THROW(m.put_file((tmp / "a.txt").string(), "https://recodex.projekty.ms.mff.cuni.cz/fm_test/a.txt"));

	//Invalid credentials
	m.set_params("codex", "re");
	EXPECT_THROW(m.put_file((tmp / "a.txt").string(), "https://recodex.projekty.ms.mff.cuni.cz/fm_test/a.txt"), fm_exception);

	fs::remove(tmp / "a.txt");
}

TEST(HttpManager, PutFileWrongURL)
{
	auto tmp = fs::temp_directory_path();
	{
		ofstream o((tmp / "b.txt").string());
		o << "testing string" << endl;
	}

	http_manager m("re", "codex");
	EXPECT_THROW(m.put_file((tmp / "b.txt").string(), "https://recodex.projekty.ms.mff.cuni.cz/fm_test_nonexist/b.txt"), fm_exception);

	m.set_params("", "");
	EXPECT_THROW(m.put_file((tmp / "b.txt").string(), "http://ps.stdin.cz/b.txt"), fm_exception);

	fs::remove(tmp / "b.txt");
}

TEST(HttpManager, PutWrongFile)
{
	auto tmp = fs::temp_directory_path();
	http_manager m("re", "codex");
	EXPECT_THROW(m.put_file((tmp / "abc5xyz.txt").string(), "https://recodex.projekty.ms.mff.cuni.cz/fm_test/abc5xyz.txt"), fm_exception);
}

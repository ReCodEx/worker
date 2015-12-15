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
	http_manager m("https://recodex.projekty.ms.mff.cuni.cz/fm_test/", "re", "codex");
	EXPECT_NO_THROW(m.get_file("test1.txt", "/tmp"));
	EXPECT_TRUE(fs::is_regular_file("/tmp/test1.txt"));
	fs::remove("/tmp/test1.txt");
}

TEST(HttpManager, GetExistingFileRedirect)
{
	http_manager m("http://recodex.projekty.ms.mff.cuni.cz/fm_test/", "re", "codex");
	EXPECT_NO_THROW(m.get_file("test2.txt", "/tmp"));
	EXPECT_TRUE(fs::is_regular_file("/tmp/test2.txt"));
	fs::remove("/tmp/test2.txt");
}

TEST(HttpManager, GetNonexistingFile)
{
	http_manager m("https://recodex.projekty.ms.mff.cuni.cz/fm_test_nonexist/", "re", "codex");
	EXPECT_THROW(m.get_file("test3.txt", "/tmp"), fm_exception);
	EXPECT_FALSE(fs::is_regular_file("/tmp/test3.txt"));
	//*
	try {
		fs::remove("/tmp/test3.txt");
	} catch(...) {}
	//*/
}

TEST(HttpManager, WrongAuthentication)
{
	http_manager m("https://recodex.projekty.ms.mff.cuni.cz/fm_test/", "codex", "re");
	EXPECT_THROW(m.get_file("test1.txt", "/tmp"), fm_exception);
	EXPECT_FALSE(fs::is_regular_file("/tmp/test1.txt"));
	fs::remove("/tmp/test1.txt");
}

TEST(HttpManager, NonexistingServer)
{
	http_manager m("http://abcd.example.com/", "a", "a");
	EXPECT_THROW(m.get_file("test1.txt", "/tmp"), fm_exception);
	EXPECT_FALSE(fs::is_regular_file("/tmp/test1.txt"));
	fs::remove("/tmp/test1.txt");
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
	http_manager m("http://httpstat.us/", "", "");
	for(auto& i : codes) {
		//std::cout << i << std::endl;
		EXPECT_THROW(m.get_file(i, "/tmp"), fm_exception);
		EXPECT_FALSE(fs::is_regular_file("/tmp/" + i));
	}
}

TEST(HttpManager, SimplePutFile)
{
	{
		ofstream o("/tmp/a.txt");
		o << "testing string" << endl;
	}

	http_manager m("https://recodex.projekty.ms.mff.cuni.cz/fm_test/", "re", "codex");
	EXPECT_NO_THROW(m.put_file("/tmp/a.txt"));

	//Invalid credentials
	m.set_params("https://recodex.projekty.ms.mff.cuni.cz/fm_test/", "codex", "re");
	EXPECT_THROW(m.put_file("/tmp/a.txt"), fm_exception);

	fs::remove("/tmp/a.txt");
}

TEST(HttpManager, PutFileWrongURL)
{
	{
		ofstream o("/tmp/b.txt");
		o << "testing string" << endl;
	}

	http_manager m("https://recodex.projekty.ms.mff.cuni.cz/fm_test_nonexist/", "re", "codex");
	EXPECT_THROW(m.put_file("/tmp/b.txt"), fm_exception);

	m.set_params("http://ps.stdin.cz/", "", "");
	EXPECT_THROW(m.put_file("/tmp/b.txt"), fm_exception);

	fs::remove("/tmp/b.txt");
}

TEST(HttpManager, PutWrongFile)
{
	http_manager m("https://recodex.projekty.ms.mff.cuni.cz/fm_test/", "re", "codex");
	EXPECT_THROW(m.put_file("/tmp/abcxzy.txt"), fm_exception);
}

TEST(HttpManager, GetSetParams)
{
	http_manager m("https://recodex.projekty.ms.mff.cuni.cz/fm_test", "re", "codex");
	//Note we expect the URL with trailing '/' character
	EXPECT_EQ(m.get_destination(), "https://recodex.projekty.ms.mff.cuni.cz/fm_test/");
	m.set_params("http://localhost/fm_test/", "", "");
	EXPECT_EQ(m.get_destination(), "http://localhost/fm_test/");
}

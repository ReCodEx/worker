#include "http_manager.h"
#include "curl_easy.h"
#include <fstream>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
namespace cu = curl;


//TODO: make sure, that remote_url_ allways ends with '/'

http_manager::http_manager(std::string remote_url, std::string username, std::string password) :
	remote_url_{remote_url}, username_{username}, password_{password}
{
}

void http_manager::get_file(std::string src_name, std::string dst_path)
{
	fs::path dest_file_path = fs::path(dst_path) / src_name;
	std::ofstream dest_file;
	dest_file.open(dest_file_path.string());
	if(!dest_file.is_open()) {
		throw fm_create_file_error("Cannot open file " + dest_file_path.string());
	}

	// Create a curl_ios object to handle the stream
	cu::curl_ios<std::ostream> writer(dest_file);
	// Pass it to the easy constructor and watch the content returned in that file!
	cu::curl_easy easy(writer);

	// Add some option to the easy handle
	easy.add<CURLOPT_URL>((remote_url_ + src_name).c_str());
	easy.add<CURLOPT_FOLLOWLOCATION>(1L);
	//Ennable support for HTTP2
	easy.add<CURLOPT_HTTP_VERSION>(CURL_HTTP_VERSION_2_0);
	//We have trusted HTTPS certificate, so set validation on
	easy.add<CURLOPT_SSL_VERIFYPEER>(1L);
	easy.add<CURLOPT_SSL_VERIFYHOST>(2L);
	//Throw exception on HTTP responses >= 400
	easy.add<CURLOPT_FAILONERROR>(1L);
	//Set HTTP authentication
	easy.add<CURLOPT_HTTPAUTH>(CURLAUTH_BASIC);
	easy.add<CURLOPT_USERPWD>((username_ + ":" + password_).c_str());

	std::string error_message;
	try {
		// Execute the request
		easy.perform();
	} catch (cu::curl_easy_exception &e) {
		error_message = "Could not fetch file from " + remote_url_+ ". Message: " + e.what();
	}

	dest_file.close();
	if(!error_message.empty()) {
		try {
			fs::remove(dest_file_path);
		} catch(...) {}
		throw fm_connection_error(error_message);
	}
}

void http_manager::put_file(std::string name)
{

}

void http_manager::set_data(std::string remote_url, std::string username, std::string password)
{
	remote_url_ = remote_url;
	username_ = username;
	password_ = password;
}
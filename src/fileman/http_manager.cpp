#include "http_manager.h"
#include <stdio.h>
#include <curl/curl.h>
#include <regex>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

//Disable warning about fopen() on Windows
#ifdef _WIN32
	#pragma warning(disable : 4996)
#endif


/* If you want run this program on Windows with libcurl as a
   DLL, you MUST also provide a read callback with CURLOPT_READFUNCTION.
   Failing to do so will give you a crash since a DLL may not use the
   variable's memory when passed in to it from an app like this. */
static size_t fread_wrapper(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fread(ptr, size, nmemb, stream);
}

//And the same for writing ...
static size_t fwrite_wrapper(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

//Nothing write callback
size_t write_callback(char *, size_t size, size_t nmemb, void *)
{
	return size * nmemb;
}

//Tweak for older libcurls
#ifndef CURL_HTTP_VERSION_2_0
	#define CURL_HTTP_VERSION_2_0 CURL_HTTP_VERSION_1_1
#endif



http_manager::http_manager(std::shared_ptr<spdlog::logger> logger) :
	logger_(logger)
{
}

http_manager::http_manager(
	const std::vector<fileman_config> &configs,
	std::shared_ptr<spdlog::logger> logger) :
	configs_(configs)
{
	if (logger != nullptr) {
		logger_ = logger;
	} else {
		//Create logger manually to avoid global registration of logger
		auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
		logger_ = std::make_shared<spdlog::logger>("cache_manager_nolog", sink);
	}
}

void http_manager::get_file(const std::string &src_name, const std::string &dst_name)
{
	CURL *curl;
	CURLcode res;
	FILE *fd;

	logger_->debug() << "Downloading file " << src_name << " to " << dst_name;

	//Open file to download
	fd = fopen(dst_name.c_str(), "wb");
	if(!fd) {
		auto message = "Cannot open file " + dst_name + " for writing.";
		logger_->warn() << message;
		throw fm_exception(message);
	}

	curl = curl_easy_init();
	if (curl) {
		//Destination URL
		curl_easy_setopt(curl, CURLOPT_URL, (src_name).c_str());

		//Set where to write data to
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
		//Use custom write function (because of Windows DLL issue)
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, fwrite_wrapper);

#ifdef _WIN32 // Windows needs to have explicitly defined certificate bundle
		curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
#endif

		//Follow redirects
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		//Ennable support for HTTP2
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
		//We have trusted HTTPS certificate, so set validation on
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		//Throw exception on HTTP responses >= 400
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		//Set HTTP authentication
		auto config = find_config(src_name);

		if (config != nullptr) {
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
			curl_easy_setopt(curl, CURLOPT_USERPWD, (config->username + ":" + config->password).c_str());
		}

		//Enable verbose for easier tracing
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		//Close file
		fclose(fd);

		//Check for errors
		if (res != CURLE_OK) {
			try {
				fs::remove(dst_name);
			} catch (...) {}
			auto error_message = "Failed to download " + src_name + " to " + dst_name +
					". Error: " + curl_easy_strerror(res);
			logger_->warn() << error_message;
			curl_easy_cleanup(curl);
			throw fm_exception(error_message);
		}

		//Always cleanup
		curl_easy_cleanup(curl);
	}
}

void http_manager::put_file(const std::string &src_name, const std::string &dst_url)
{
	fs::path source_file(src_name);
	CURL *curl;
	CURLcode res;
	FILE *fd;

	logger_->debug() << "Uploading file " << src_name << " to " << dst_url;

	//Open file to upload
	fd = fopen(src_name.c_str(), "rb");
	if (!fd) {
		auto message = "Cannot open file " + src_name + " for reading.";
		logger_->warn() << message;
		throw fm_exception(message);
	}

	//Get the file size
	auto filesize = fs::file_size(source_file);

	curl = curl_easy_init();
	if (curl) {
		//Destination URL
		curl_easy_setopt(curl, CURLOPT_URL, dst_url.c_str());

		//Upload mode
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		//Set where to read data from
		curl_easy_setopt(curl, CURLOPT_READDATA, fd);
		//Use custom read function (because of Windows DLL issue)
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, fread_wrapper);

		//Drop output - the page after put request
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

		//Better give size of uploaded file
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)filesize);

#ifdef _WIN32 // Windows needs to have explicitly defined certificate bundle
		curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
#endif

		//Follow redirects
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		//Ennable support for HTTP2
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
		//Trusted HTTPS certificate is not problem (see Let's Encrypt project), so set validation on
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		//Throw exception on HTTP responses >= 400
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		//Set HTTP authentication
		auto config = find_config(dst_url);

		if (config != nullptr) {
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
			curl_easy_setopt(curl, CURLOPT_USERPWD, (config->username + ":" + config->password).c_str());
		}

		//Enable verbose for easier tracing
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		//Close file
		fclose(fd);

		//Check for errors
		if (res != CURLE_OK) {
			auto message = "Failed to upload " + src_name + " to " + dst_url +
					". Error: " + curl_easy_strerror(res);
			logger_->warn() << message;
			throw fm_exception(message);
		}

		//Always cleanup
		curl_easy_cleanup(curl);
	}
}

const fileman_config *http_manager::find_config(const std::string &url) const
{
	for (auto it = std::begin(configs_); it != std::end(configs_); ++it) {
		if (url.compare(0, it->remote_url.size(), it->remote_url) == 0) {
			return &(*it);
		}
	}

	return nullptr;
}

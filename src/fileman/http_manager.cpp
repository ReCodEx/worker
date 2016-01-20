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

//Tweak for older libcurls
#ifndef CURL_HTTP_VERSION_2_0
	#define CURL_HTTP_VERSION_2_0 CURL_HTTP_VERSION_1_1
#endif



http_manager::http_manager(std::shared_ptr<spdlog::logger> logger) :
	logger_(logger)
{
}

http_manager::http_manager(
	const fileman_config &config,
	std::shared_ptr<spdlog::logger> logger) :
	config_(config)
{
	if (logger != nullptr) {
		logger_ = logger;
	} else {
		//Create logger manually to avoid global registration of logger
		auto sink = std::make_shared<spdlog::sinks::null_sink_st>();
		logger_ = std::make_shared<spdlog::logger>("cache_manager_nolog", sink);
	}
}

void http_manager::get_file(const std::string &src_name, const std::string &dst_path)
{
	CURL *curl;
	CURLcode res;
	FILE *fd;

	logger_->debug() << "Downloading file " << src_name << " to " << dst_path;

	//Open file to download
	fd = fopen(dst_path.c_str(), "wb");
	if(!fd) {
		auto message = "Cannot open file " + dst_path + " for writing.";
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
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		curl_easy_setopt(curl, CURLOPT_USERPWD, (config_.username + ":" + config_.password).c_str());

		//Enable verbose for easier tracing
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		//Close file
		fclose(fd);

		//Check for errors
		if (res != CURLE_OK) {
			try {
				fs::remove(dst_path);
			} catch (...) {}
			auto error_message = "Failed to download " + src_name + " to " + dst_path +
					". Error: " + curl_easy_strerror(res);
			logger_->warn() << error_message;
			curl_easy_cleanup(curl);
			throw fm_exception(error_message);
		}

		//Always cleanup
		curl_easy_cleanup(curl);
	}
}

void http_manager::put_file(const std::string &src, const std::string &dst)
{
	fs::path source_file(src);
	CURL *curl;
	CURLcode res;
	FILE *fd;

	logger_->debug() << "Uploading file " << src << " to " << dst;

	//Open file to upload
	fd = fopen(src.c_str(), "rb");
	if (!fd) {
		auto message = "Cannot open file " + src + " for reading.";
		logger_->warn() << message;
		throw fm_exception(message);
	}

	//Get the file size
	auto filesize = fs::file_size(source_file);

	curl = curl_easy_init();
	if (curl) {
		//Destination URL
		curl_easy_setopt(curl, CURLOPT_URL, dst.c_str());

		//Upload mode
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		//Set where to read data from
		curl_easy_setopt(curl, CURLOPT_READDATA, fd);
		//Use custom read function (because of Windows DLL issue)
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, fread_wrapper);

		//Better give size of uploaded file
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)filesize);

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
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		curl_easy_setopt(curl, CURLOPT_USERPWD, (config_.username + ":" + config_.password).c_str());

		//Enable verbose for easier tracing
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		//Close file
		fclose(fd);

		//Check for errors
		if (res != CURLE_OK) {
			auto message = "Failed to upload " + src + " to " + dst +
					". Error: " + curl_easy_strerror(res);
			logger_->warn() << message;
			throw fm_exception(message);
		}

		//Always cleanup
		curl_easy_cleanup(curl);
	}
}

#include "http_manager.h"
#include <stdio.h>
#include <curl/curl.h>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

// Disable warning about fopen() on Windows
#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

namespace
{

	/* If you want run this program on Windows with libcurl as a
	   DLL, you MUST also provide a read callback with CURLOPT_READFUNCTION.
	   Failing to do so will give you a crash since a DLL may not use the
	   variable's memory when passed in to it from an app like this. */
	static std::size_t fread_wrapper(void *ptr, std::size_t size, std::size_t nmemb, FILE *stream)
	{
		return fread(ptr, size, nmemb, stream);
	}

	// And the same for writing ...
	static std::size_t fwrite_wrapper(void *ptr, std::size_t size, std::size_t nmemb, FILE *stream)
	{
		return fwrite(ptr, size, nmemb, stream);
	}

	// Nothing write callback
	std::size_t write_callback(char *, std::size_t size, std::size_t nmemb, void *)
	{
		return size * nmemb;
	}

} // namespace

// Tweak for older libcurls
#ifndef CURL_HTTP_VERSION_2_0
#define CURL_HTTP_VERSION_2_0 CURL_HTTP_VERSION_1_1
#endif


http_manager::http_manager(std::shared_ptr<spdlog::logger> logger) : logger_(logger)
{
}

http_manager::http_manager(const std::vector<fileman_config> &configs, std::shared_ptr<spdlog::logger> logger)
	: configs_(configs), logger_(logger)
{
	if (logger_ == nullptr) { logger_ = helpers::create_null_logger(); }
}

void http_manager::get_file(const std::string &src_name, const std::string &dst_name)
{
	logger_->debug("Downloading file {} to {}", src_name, dst_name);

	// Open file to download
	std::unique_ptr<FILE, decltype(&fclose)> fd = {fopen(dst_name.c_str(), "wb"), fclose};
	if (!fd.get()) {
		auto message = "Cannot open file " + dst_name + " for writing.";
		logger_->warn(message);
		throw fm_exception(message);
	}

	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl = {curl_easy_init(), curl_easy_cleanup};
	if (curl.get()) {
		// Destination URL
		curl_easy_setopt(curl.get(), CURLOPT_URL, (src_name).c_str());

		// Set where to write data to
		curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, fd.get());
		// Use custom write function (because of Windows DLL issue)
		curl_easy_setopt(curl.get(), CURLOPT_READFUNCTION, fwrite_wrapper);

#ifdef _WIN32 // Windows needs to have explicitly defined certificate bundle
		curl_easy_setopt(curl.get(), CURLOPT_CAINFO, "curl-ca-bundle.crt");
#endif

		// Follow redirects
		curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
		// Ennable support for HTTP2
		curl_easy_setopt(curl.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
		// We have trusted HTTPS certificate, so set validation on
		curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
		// Throw exception on HTTP responses >= 400
		curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);

		// Set HTTP authentication
		auto config = find_config(src_name);

		if (config != nullptr) {
			curl_easy_setopt(curl.get(), CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
			curl_easy_setopt(curl.get(), CURLOPT_USERPWD, (config->username + ":" + config->password).c_str());
		}

		// Enable verbose for easier tracing
		// curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);

		CURLcode res = curl_easy_perform(curl.get());

		// Check for errors
		if (res != CURLE_OK) {
			try {
				fs::remove(dst_name);
			} catch (...) {
			}
			long response_code;
			curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response_code);
			auto error_message = "Failed to download " + src_name + " to " + dst_name + ". Error: (" +
				std::to_string(response_code) + ") " + curl_easy_strerror(res);
			logger_->warn(error_message);
			throw fm_exception(error_message);
		}

		// set write permissions to downloaded file
		try {
			fs::permissions(fs::path(dst_name),
				fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write,
				fs::perm_options::add);
		} catch (fs::filesystem_error &e) {
			auto message = "Failed to set write permissions on '" + dst_name + "'. Error: " + e.what();
			logger_->warn(message);
			throw fm_exception(message);
		}
	}
}

void http_manager::put_file(const std::string &src_name, const std::string &dst_url)
{
	fs::path source_file(src_name);

	logger_->debug("Uploading file {} to {}", src_name, dst_url);

	// Open file to upload
	std::unique_ptr<FILE, decltype(&fclose)> fd = {fopen(src_name.c_str(), "rb"), fclose};
	if (!fd.get()) {
		auto message = "Cannot open file " + src_name + " for reading.";
		logger_->warn(message);
		throw fm_exception(message);
	}

	// Get the file size
	auto filesize = fs::file_size(source_file);

	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl = {curl_easy_init(), curl_easy_cleanup};
	if (curl.get()) {
		// Destination URL
		curl_easy_setopt(curl.get(), CURLOPT_URL, dst_url.c_str());

		// Upload mode
		curl_easy_setopt(curl.get(), CURLOPT_UPLOAD, 1L);

		// Set where to read data from
		curl_easy_setopt(curl.get(), CURLOPT_READDATA, fd.get());
		// Use custom read function (because of Windows DLL issue)
		curl_easy_setopt(curl.get(), CURLOPT_READFUNCTION, fread_wrapper);

		// Drop output - the page after put request
		curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_callback);

		// Better give size of uploaded file
		curl_easy_setopt(curl.get(), CURLOPT_INFILESIZE_LARGE, (curl_off_t) filesize);

#ifdef _WIN32 // Windows needs to have explicitly defined certificate bundle
		curl_easy_setopt(curl.get(), CURLOPT_CAINFO, "curl-ca-bundle.crt");
#endif

		// Follow redirects
		curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
		// Ennable support for HTTP2
		curl_easy_setopt(curl.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
		// Trusted HTTPS certificate is not problem (see Let's Encrypt project), so set validation on
		curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
		// Throw exception on HTTP responses >= 400
		curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);

		// Set HTTP authentication
		auto config = find_config(dst_url);

		if (config != nullptr) {
			curl_easy_setopt(curl.get(), CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
			curl_easy_setopt(curl.get(), CURLOPT_USERPWD, (config->username + ":" + config->password).c_str());
		}

		// Enable verbose for easier tracing
		// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		CURLcode res = curl_easy_perform(curl.get());

		// Check for errors
		if (res != CURLE_OK) {
			long response_code;
			curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response_code);
			auto message = "Failed to upload " + src_name + " to " + dst_url + ". Error: (" +
				std::to_string(response_code) + ") " + curl_easy_strerror(res);
			logger_->warn(message);
			throw fm_exception(message);
		}
	}
}

const fileman_config *http_manager::find_config(const std::string &url) const
{
	for (const auto &item : configs_) {
		if (url.compare(0, item.remote_url.size(), item.remote_url) == 0) { return &item; }
	}

	return nullptr;
}

#include "sandbox_base.h"

#include <filesystem>
#include "helpers/filesystem.h"
#include "helpers/logger.h"

sandbox_base::sandbox_base(std::shared_ptr<sandbox_config> sandbox_config,
	sandbox_limits limits,
	std::size_t id,
	const std::string &temp_dir,
	const std::string &data_dir,
	const std::string &sandbox_binary,
	std::shared_ptr<spdlog::logger> logger)
	: sandbox_config_(sandbox_config), limits_(limits), logger_(logger), id_(id), data_dir_(data_dir),
	  sandbox_binary_(sandbox_binary)
{
	if (logger_ == nullptr) {
		// logger must not be empty, place dummy instead
		logger_ = helpers::create_null_logger();
	}

	if (sandbox_config_ == nullptr) { log_and_throw(logger_, "No sandbox configuration provided."); }

	if (data_dir_ == "") { logger_->info("Empty data directory for moving to sandbox."); }

	// Set backup limit (for killing isolate if it hasn't finished yet)
	max_timeout_ = limits_.wall_time > limits_.cpu_time ? limits_.wall_time : limits_.cpu_time;
	max_timeout_ += 300; // plus 5 minutes (for short tasks)
	max_timeout_ *= 1.2; // 20% time more than necessary (better have some spare time)

	temp_dir_ = (fs::path(temp_dir) / std::to_string(id_)).string();
	try {
		fs::create_directories(temp_dir_);
	} catch (fs::filesystem_error &e) {
		log_and_throw(logger_, "Failed to create temp directory for the sandbox. Error: ", e.what());
	}
}

sandbox_results sandbox_base::execute_in_sandbox(const std::string &binary, const std::vector<std::string> &arguments)
{
	try {
		sandbox_init();
	} catch (...) {
		fs::remove_all(temp_dir_);
		throw;
	}

	// move data to the sandboxed directory
	if (data_dir_ != "") { move_or_throw(logger_, data_dir_, sandboxed_dir_); }

	try {
		// run the sandbox (virtual method implemented in child class)
		sandbox_run(binary, arguments);

	} catch (const std::exception &e_run) {
		try {
			// on errors also move data from the sandboxed directory back to data directory
			// but we need to do it safely, so the original exception is rethrown after this
			if (data_dir_ != "") { move_or_throw(logger_, sandboxed_dir_, data_dir_); }
		} catch (const std::exception &e) {
			logger_->error("When ", sandbox_binary_, " execution failed... ", e.what());
		}

		// rethrow the original exception when data are saved
		throw e_run;
	}

	// move data from the sandboxed directory back to data directory (regular case)
	if (data_dir_ != "") { move_or_throw(logger_, sandboxed_dir_, data_dir_); }

	auto results = extract_results(); // this is also virtual method

	sandbox_cleanup();
	fs::remove_all(temp_dir_);

	return results;
}


/*
 * Helper functions
 */

void move_or_throw(std::shared_ptr<spdlog::logger> logger, const std::string &from, const std::string &to)
{
	try {
		helpers::copy_directory(from, to, true); // true = skip symlinks for security reasons
	} catch (fs::filesystem_error &e) {
		log_and_throw(logger, "Failed moving ", from, " to ", to, ", error: ", e.what());
	}

	try {
		fs::remove_all(from);
	} catch (fs::filesystem_error &) {
		// deliberately ignore this error
	}
}

/*
 * Helper class sandbox_log_pipe
 */

sandbox_log_pipe::sandbox_log_pipe(std::shared_ptr<spdlog::logger> logger)
{
	logger_ = logger;
	int fds[2];
	if (pipe(fds) < 0) { log_and_throw(logger_, "Cannot create pipe: %m"); }
	read_fd_ = fds[0];
	write_fd_ = fds[1];
}

sandbox_log_pipe::~sandbox_log_pipe()
{
	if (read_fd_ >= 0) { close(read_fd_); }
	if (write_fd_ >= 0) { close(write_fd_); }
}

void sandbox_log_pipe::child_dup_to_fd(int fd)
{
	// Call in child process
	dup2(write_fd_, fd);
	close(read_fd_);
	close(write_fd_);
	read_fd_ = write_fd_ = -1;
}

boost::iostreams::stream<boost::iostreams::file_descriptor_source> sandbox_log_pipe::parent_read_stream()
{
	// Call in parent process
	close(write_fd_);
	write_fd_ = -1;
	return boost::iostreams::stream<boost::iostreams::file_descriptor_source>(
		read_fd_, boost::iostreams::never_close_handle);
}

#include "isolate_sandbox.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>


isolate_sandbox::isolate_sandbox(sandbox_limits limits, size_t id, std::shared_ptr<spdlog::logger> logger) :
	limits_(limits), id_(id), isolate_binary_("/usr/local/bin/isolate")
{
	if (logger != nullptr) {
		logger_ = logger;
	} else {
		//Create logger manually to avoid global registration of logger
		auto sink = std::make_shared<spdlog::sinks::stderr_sink_st>();
		logger_ = std::make_shared<spdlog::logger>("cache_manager_nolog", sink);
		//Set loglevel to 'off' cause no logging
		logger_->set_level(spdlog::level::off);
	}

	isolate_init();
}

isolate_sandbox::~isolate_sandbox()
{

}

task_results isolate_sandbox::run(const std::string &binary, const std::string &arguments)
{
	return task_results();
}

void isolate_sandbox::isolate_init()
{
	int fd[2];
	pid_t childpid;

	//Create unnamend pipe
	if(pipe(fd) == -1) {
		auto message = std::string("Cannot create pipe: ") + strerror(errno);
		logger_->warn() << message;
		throw sandbox_exception(message);
	}

	childpid = fork();

	switch (childpid) {
	case -1:
		{
			auto message = std::string("Fork failed: ") + strerror(errno);
			logger_->warn() << message;
			throw sandbox_exception(message);
		}
		break;
	case 0:
		//---Child---
		//Close up output side of pipe
		close(fd[0]);

		//Close stdout, duplicate the input side of pipe to stdout
		dup2(fd[1], 1);

		//Redirect stderr to /dev/null file
		int devnull;
		devnull = open("/dev/null", O_WRONLY);
		if (devnull == -1) {
			auto message = "Cannot open /dev/null file for writing.";
			logger_->warn() << message;
			throw sandbox_exception(message);
		}
		dup2(devnull, 2);

		//Exec isolate init command
		const char *args[6];
		args[0] = isolate_binary_.c_str();
		args[1] = "--cg";
		args[2] = "-b";
		args[3] = std::to_string(id_).c_str();
		args[4] = "--init";
		args[5] = NULL;
		//const_cast is ugly, but this is working with C code - execv does not modify its arguments
		execvp(isolate_binary_.c_str(), const_cast<char **>(args));

		//Never reached
		{
			auto message = std::string("Exec returned to child: ") + strerror(errno);
			logger_->warn() << message;
			throw sandbox_exception(message);
		}
		break;
	default:
		//---Parent---
		//Close up input side of pipe
		close(fd[1]);

		char buf[256];
		int ret;
		while ((ret = read(fd[0], (void *)buf, 256)) > 0) {
			if (buf[ret - 1] == '\n') {
				buf[ret - 1] = '\0';
			}
			sandboxed_dir_ += std::string(buf);
		}
		if(ret == -1) {
			auto message = "Read from pipe error.";
			logger_->warn() << message;
			throw sandbox_exception(message);
		}

		int status;
		waitpid(childpid, &status, 0);
		if(WEXITSTATUS(status) != 0) {
			auto message = "Isolate init error. Return value: " + std::to_string(WEXITSTATUS(status));
			logger_->warn() << message;
			throw sandbox_exception(message);
		}
		logger_->debug() << "Isolate initialized in " << sandboxed_dir_;
		break;
	}
}


#ifndef CODEX_WORKER_ISOEVAL_CORE_HPP
#define CODEX_WORKER_ISOEVAL_CORE_HPP

#include <iostream>
#include <yaml-cpp/yaml.h>
#include <memory>
#include "spdlog/spdlog.h"
#include <curl/curl.h>
#include <vector>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


// Our very own code includes
#include "config/worker_config.h"
#include "broker_connection.h"
#include "connection_proxy.h"
#include "tasks/job.h"


/**
 *
 */
class isoeval_core {
public:
	isoeval_core() = delete;
	isoeval_core(const isoeval_core &source) = delete;
	isoeval_core& operator=(const isoeval_core &source) = delete;
	isoeval_core(const isoeval_core &&source) = delete;
	isoeval_core& operator=(const isoeval_core &&source) = delete;

	isoeval_core(std::vector<std::string> args);
	~isoeval_core();

	void run();
	void push_job_config(std::string filename);
private:

	void log_init();
	void curl_init();
	void broker_init();
	void force_exit(std::string msg);
	void parse_params();
	void load_config();

	std::vector<std::string> args_;
	YAML::Node config_;
	job job_;
	std::string log_filename_;
	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<broker_connection<connection_proxy, receive_task_callback>> broker_;
};

#endif //CODEX_WORKER_ISOEVAL_CORE_HPP

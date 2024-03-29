#ifndef RECODEX_WORKER_ISOEVAL_CORE_HPP
#define RECODEX_WORKER_ISOEVAL_CORE_HPP

#include "spdlog/spdlog.h"
#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <filesystem>

#include <boost/program_options.hpp>

#include "broker_connection.h"
#include "config/log_config.h"
#include "config/worker_config.h"
#include "connection_proxy.h"
#include "fileman/fallback_file_manager.h"
#include "fileman/file_manager_interface.h"
#include "job/job_receiver.h"
#include "job/job_evaluator.h"

namespace fs = std::filesystem;

/**
 * Main class of the whole program.
 * It handles the creation and destruction of all services.
 */
class worker_core
{
public:
	worker_core() = delete;
	worker_core(const worker_core &source) = delete;
	worker_core &operator=(const worker_core &source) = delete;
	worker_core(const worker_core &&source) = delete;
	worker_core &operator=(const worker_core &&source) = delete;

	/**
	 * There is only one constructor, which should get cmd parameters.
	 * Constructor initializes all variables and structures, parses cmd parameters and load configuration.
	 * @param args Cmd line parameters
	 */
	worker_core(std::vector<std::string> args);

	/**
	 * All structures which need to be explicitly destructed or unitialized should do it now.
	 */
	~worker_core();

	/**
	 * Constructors initializes all things,	all we have to do now is launch all the fun.
	 * This method creates separate thread for broker_connection and starts job_evaluator service.
	 */
	void run();

private:
	/**
	 * Setup all things around spdlog logger, creates log path/file if not existing.
	 */
	void log_init();

	/**
	 * Globally initializes CURL
	 */
	void curl_init();

	/**
	 * CURL cleanup
	 */
	void curl_fini();

	/**
	 * Construct and setup broker connection.
	 * This function does not run broker in separated thread,
	 * this is done in run() function.
	 */
	void broker_init();

	/**
	 * Exit whole application with return code 1.
	 * @param msg string which is copied to stderr and logger if initialized.
	 */
	void force_exit(const std::string &msg = "");

	/**
	 * Parse cmd line params given in constructor.
	 */
	void parse_params();

	/**
	 * Load default worker configuration from default config file
	 * or from file given in cmd parameters.
	 */
	void load_config();

	/**
	 * Initialize file manager, there is only one on whole instance of worker.
	 * Hostname of remote server can be changed.
	 */
	void fileman_init();

	/**
	 * Job receiver and evaluator construction and initialization.
	 */
	void receiver_init();

	/**
	 * Initialize working directory of whole worker.
	 */
	void filesystem_init();


	// PRIVATE DATA MEMBERS
	/** Cmd line parameters */
	std::vector<std::string> args_;

	/** Filename of default configuration of worker */
	std::string config_filename_;
	/** Loaded worker configuration */
	std::shared_ptr<worker_config> config_;

	/** Working directory of this instance of worker */
	fs::path working_directory_;

	/** Pointer to logger */
	std::shared_ptr<spdlog::logger> logger_;

	/** File manager that works with a remote file storage */
	std::shared_ptr<file_manager_interface> remote_fm_;
	/** File manager that works with a local cache */
	std::shared_ptr<file_manager_interface> cache_fm_;

	/** Handles evaluation and all things around */
	std::shared_ptr<job_receiver> job_receiver_;

	/** Handles connection to broker, receiving submission and pushing results */
	std::shared_ptr<broker_connection<connection_proxy>> broker_;

	/** A ZeroMQ context */
	std::shared_ptr<zmq::context_t> zmq_context_;
};

#endif // RECODEX_WORKER_ISOEVAL_CORE_HPP

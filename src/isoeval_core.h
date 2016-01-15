#ifndef CODEX_WORKER_ISOEVAL_CORE_HPP
#define CODEX_WORKER_ISOEVAL_CORE_HPP

#include <iostream>
#include <yaml-cpp/yaml.h>
#include <memory>
#include "spdlog/spdlog.h"
#include <curl/curl.h>
#include <vector>
#include <functional>
#include <thread>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
namespace fs = boost::filesystem;


// Our very own code includes
#include "config/worker_config.h"
#include "config/log_config.h"
#include "broker_connection.h"
#include "connection_proxy.h"
#include "job_receiver.h"
#include "fileman/file_manager_base.h"
#include "fileman/file_manager.h"


/**
 * Main class of whole program.
 * It handles all creation and destruction of all used parts.
 * And of course run all those parts.
 */
class isoeval_core
{
public:
	isoeval_core () = delete;

	isoeval_core (const isoeval_core &source) = delete;

	isoeval_core &operator= (const isoeval_core &source) = delete;

	isoeval_core (const isoeval_core &&source) = delete;

	isoeval_core &operator= (const isoeval_core &&source) = delete;

	/**
	 * There is only one constructor, which should get cmd parameters.
	 * Constructor initializes all variables and structures, parses cmd parameters and load configuration.
	 * @param args Cmd line parameters
	 */
	isoeval_core (std::vector<std::string> args);

	/**
	 * All structures which need to be explicitly destructed or unitialized should do it now.
	 */
	~isoeval_core ();

	/**
	 * Constructors initializes all things,	all we have to do now is launch all the fun.
	 * This method creates separate thread for broker_connection and starts job_evaluator service.
	 */
	void run ();

private:

	/**
	 * Setup all things around spdlog logger, creates log path/file if not existing.
	 */
	void log_init ();

	/**
	 * Globally initializes CURL
	 */
	void curl_init ();

	/**
	 * CURL cleanup
	 */
	void curl_fini ();

	/**
	 * Construct and setup broker connection.
	 * This function does not run broker in separated thread,
	 * this is done in run() function.
	 */
	void broker_init ();

	/**
	 * Exit whole application with return code 1.
	 * @param msg string which is copied to stderr and logger if initialized.
	 */
	void force_exit (std::string msg = "");

	/**
	 * Parse cmd line params given in constructor.
	 */
	void parse_params ();

	/**
	 * Load default worker configuration from default config file
	 * or from file given in cmd parameters.
	 */
	void load_config ();

	/**
	 * Initialize file manager, there is only one on whole instance of worker.
	 * Hostname of remote server can be changed.
	 */
	void fileman_init ();

	/**
	 * Job receiver and evaluator construction and initialization.
	 */
	void receiver_init ();


	// PRIVATE DATA MEMBERS
	/** Cmd line parameters */
	std::vector<std::string> args_;

	/** Filename of default configuration of worker */
	std::string config_filename_;

	/** Loaded worker configuration */
	std::shared_ptr<worker_config> config_;

	/** Pointer to logger */
	std::shared_ptr<spdlog::logger> logger_;

	/** File manager for submission archives which need no caching */
	std::shared_ptr<file_manager_base> submission_fileman_;
	/** File manager which is used to download and upload needed files */
	std::shared_ptr<file_manager_base> fileman_;

	/** Handles evaluation and all things around */
	std::shared_ptr<job_receiver> job_receiver_;

	/** Handles connection to broker, receiving submission and pushing results */
	std::shared_ptr<broker_connection<connection_proxy>> broker_;

	/** A ZeroMQ context */
	zmq::context_t zmq_context_;
};

#endif //CODEX_WORKER_ISOEVAL_CORE_HPP

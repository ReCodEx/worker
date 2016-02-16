#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "isoeval_core.h"
#include "fileman/cache_manager.h"
#include "fileman/http_manager.h"
#include "job/job_receiver.h"


isoeval_core::isoeval_core(std::vector<std::string> args)
	: args_(args), config_filename_("config.yml"), working_directory_(fs::temp_directory_path() / "isoeval"),
	  logger_(nullptr), remote_fm_(nullptr), cache_fm_(nullptr), job_receiver_(nullptr), broker_(nullptr)
{
	// parse cmd parameters
	parse_params();
	// load configuration from yaml file
	load_config();
	// initialize working directory
	filesystem_init();
	// initialize logger
	log_init();
	// initialize curl
	curl_init();
	// construct and setup broker connection
	broker_init();
	// construct filemanagers
	fileman_init();
	// evaluator initialization
	receiver_init();
}

isoeval_core::~isoeval_core()
{
	// curl finalize
	curl_fini();
}

void isoeval_core::run()
{
	// connect broker_connection to real broker server
	broker_->connect();
	// start execution thread which will be receiving jobs
	std::thread broker_thread;
	logger_->info() << "Trying to create broker connection thread.";
	try {
		broker_thread = std::thread(std::bind(&broker_connection<connection_proxy>::receive_tasks, broker_));
	} catch (std::system_error &e) {
		logger_->emerg() << "Broker connection thread cannot be started: " << e.what();
		return;
	}
	logger_->info() << "Broker connection thread created succesfully.";

	logger_->info() << "Job receiver will now start receiving.";
	job_receiver_->start_receiving();

	broker_thread.join();
	return;
}

void isoeval_core::parse_params()
{
	using namespace boost::program_options;

	// Declare the supported options.
	options_description desc("Allowed options for IsoEval");
	desc.add_options()("help,h", "Writes this help message to stderr")(
		"config,c", value<std::string>(), "Set default configuration of this program");

	variables_map vm;
	try {
		store(command_line_parser(args_).options(desc).run(), vm);
		notify(vm);
	} catch (std::exception &e) {
		force_exit("Error in loading a parameter: " + std::string(e.what()));
	}


	// Evaluate all information from command line
	if (vm.count("help")) {
		std::cerr << desc << std::endl;
		force_exit();
	}

	if (vm.count("config")) {
		config_filename_ = vm["config"].as<std::string>();
	}

	return;
}

void isoeval_core::load_config()
{
	try {
		YAML::Node config_yaml = YAML::LoadFile(config_filename_);
		config_ = std::make_shared<worker_config>(config_yaml);
	} catch (std::exception &e) {
		force_exit("Error loading config file: " + std::string(e.what()));
	}

	// if there was working directory defined, than modify it accordingly...
	working_directory_ = config_->get_working_directory();

	return;
}

void isoeval_core::force_exit(std::string msg)
{
	// write to log
	if (msg != "") {
		if (logger_ != nullptr) {
			logger_->emerg() << msg;
		}
		std::cerr << msg << std::endl;
	}

	exit(1);
}

void isoeval_core::log_init()
{
	auto log_conf = config_->get_log_config();

	// Set up logger
	// Try to create target directory for logs
	auto path = fs::path(log_conf.log_path);
	try {
		if (!fs::is_directory(path)) {
			fs::create_directories(path);
		}
	} catch (fs::filesystem_error &e) {
		std::cerr << "Logger: " << e.what() << std::endl;
		throw;
	}

	// Create and register logger
	try {
		// Create multithreaded rotating file sink. Max filesize is 1024 * 1024 and we save 5 newest files.
		auto rotating_sink =
			std::make_shared<spdlog::sinks::rotating_file_sink_mt>((path / log_conf.log_basename).string(),
				log_conf.log_suffix,
				log_conf.log_file_size,
				log_conf.log_files_count,
				true);
		// Set queue size for asynchronous logging. It must be a power of 2.
		spdlog::set_async_mode(1048576);
		// Make log with name "logger"
		logger_ = std::make_shared<spdlog::logger>("logger", rotating_sink);
		// Set logging level to debug
		logger_->set_level(log_config::get_level(log_conf.log_level));
		// Print header to log
		logger_->emerg() << "------------------------------";
		logger_->emerg() << "    Started ReCodEx worker";
		logger_->emerg() << "------------------------------";
	} catch (spdlog::spdlog_ex &e) {
		std::cerr << "Logger: " << e.what() << std::endl;
		throw;
	}

	return;
}

void isoeval_core::curl_init()
{
	// Globally init curl library

	logger_->info() << "Initializing CURL...";
	curl_global_init(CURL_GLOBAL_DEFAULT);
	logger_->info() << "CURL initialized.";

	return;
}

void isoeval_core::curl_fini()
{
	// Clean after curl library
	logger_->info() << "Cleanup after CURL...";
	curl_global_cleanup();
	logger_->info() << "CURL cleaned.";

	return;
}

void isoeval_core::broker_init()
{
	logger_->info() << "Initializing broker connection...";
	auto broker_proxy = std::make_shared<connection_proxy>(zmq_context_);

	broker_ = std::make_shared<broker_connection<connection_proxy>>(*config_, broker_proxy, logger_);
	logger_->info() << "Broker connection initialized.";

	return;
}

void isoeval_core::fileman_init()
{
	logger_->info() << "Initializing file managers...";
	auto fileman_conf = config_->get_filemans_configs();
	remote_fm_ = std::make_shared<http_manager>(fileman_conf, logger_);
	cache_fm_ = std::make_shared<cache_manager>(config_->get_cache_dir(), logger_);
	logger_->info() << "File managers initialized.";

	return;
}

void isoeval_core::receiver_init()
{
	logger_->info() << "Initializing job receiver and evaluator...";
	auto evaluator = std::make_shared<job_evaluator>(logger_, config_, remote_fm_, cache_fm_, working_directory_);
	job_receiver_ = std::make_shared<job_receiver>(zmq_context_, evaluator, logger_);
	logger_->info() << "Job receiver and evaluator initialized.";
	return;
}

void isoeval_core::filesystem_init()
{
	try {
		fs::create_directories(working_directory_);
	} catch (fs::filesystem_error &e) {
		throw e;
	}
}

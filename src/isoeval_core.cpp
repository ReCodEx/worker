#include "isoeval_core.h"

isoeval_core::isoeval_core(std::vector<std::string> args)
	: args_(args), job_(), logger_(nullptr), broker_(nullptr),
	  config_filename_("config.yml"), log_config_()
{
	parse_params();
	curl_init();
	log_init();
	load_config();
	broker_init();
}

isoeval_core::~isoeval_core()
{
	//Clean after curl library
	curl_global_cleanup();
}

void isoeval_core::run()
{
	broker_->connect();
	broker_->receive_tasks();

	return;
}

void isoeval_core::push_job_config(std::string filename)
{
	return;
}

void isoeval_core::parse_params()
{
	using namespace boost::program_options;

	// Declare the supported options.
	options_description desc("Allowed options for IsoEval");
	desc.add_options()
		("help,h", "Writes this help message to stderr")
		("config,c", value<std::string>(), "Set default configuration of this program")
		("log-path,l", value<std::string>(), "Set path to logging folder");

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

	if (vm.count("log-path")) {
		log_config_.log_path_ = vm["log-path"].as<std::string>();
	}

	return;
}

void isoeval_core::load_config()
{
	try {
		config_ = YAML::LoadFile("config.yml");
	} catch (YAML::Exception e) {
		force_exit("Error loading config file: " + std::string(e.what()));
	}

	return;
}

void isoeval_core::force_exit(std::string msg)
{
	// write to log
	if (msg != "") {
		if (logger_ != nullptr) {}
		std::cerr << msg << std::endl;
	}

	exit(1);
}

void isoeval_core::log_init()
{
	//Set up logger
	//Try to create target directory for logs
	auto path = fs::path(log_config_.log_path_);
	try {
		if(!fs::is_directory(path)) {
			fs::create_directories(path);
		}
	} catch(fs::filesystem_error &e) {
		std::cerr << "Logger: " << e.what() << std::endl;
		throw;
	}

	//Create and register logger
	try {
		//Create multithreaded rotating file sink. Max filesize is 1024 * 1024 and we save 5 newest files.
		auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((path / log_config_.log_basename_).string(),
								log_config_.log_suffix_, log_config_.log_file_size, log_config_.log_files_count, false);
		//Set queue size for asynchronous logging. It must be a power of 2.
		spdlog::set_async_mode(1048576);
		//Make log with name "logger"
		logger_ = std::make_shared<spdlog::logger>("logger", rotating_sink);
		//Set logging level to debug
		logger_->set_level(log_config_.log_level);
		//Print header to log
		logger_->emerg() << "------------------------------";
		logger_->emerg() << "    Started ReCodEx worker";
		logger_->emerg() << "------------------------------";
	} catch(spdlog::spdlog_ex &e) {
		std::cerr << "Logger: " << e.what() << std::endl;
		throw;
	}

	return;
}

void isoeval_core::curl_init()
{
	//Globally init curl library
	curl_global_init(CURL_GLOBAL_DEFAULT);

	return;
}

void isoeval_core::broker_init()
{
	worker_config config(config_);
	connection_proxy proxy;

	broker_ = std::make_shared<broker_connection<connection_proxy, receive_task_callback>>(config, &proxy, logger_);

	return;
}

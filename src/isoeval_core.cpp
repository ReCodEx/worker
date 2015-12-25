#include "isoeval_core.h"

isoeval_core::isoeval_core(std::vector<std::string> args)
	: args_(args), job_(), logger_(nullptr), broker_(nullptr)
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
	// use boost::program_options
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
	if (logger_ != nullptr) {}
	std::cerr << msg << std::endl;

	exit(1);
}

void isoeval_core::log_init()
{
	//Params which will be configured - will be implemented later
	std::string log_path = "/tmp/recodex_log/";
	std::string log_basename = "worker";
	std::string log_suffix = "log";
	spdlog::level::level_enum log_level = spdlog::level::debug;
	int log_file_size = 1024 * 1024;
	int log_files_count = 3;

	//Set up logger
	//Try to create target directory for logs
	auto path = fs::path(log_path);
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
		auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((path / log_basename).string(),
								log_suffix, log_file_size, log_files_count, false);
		//Set queue size for asynchronous logging. It must be a power of 2.
		spdlog::set_async_mode(1048576);
		//Make log with name "logger"
		logger_ = std::make_shared<spdlog::logger>("logger", rotating_sink);
		//Set logging level to debug
		logger_->set_level(log_level);
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

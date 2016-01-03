#ifndef CODEX_WORKER_FILEMAN_CONFIG_H
#define CODEX_WORKER_FILEMAN_CONFIG_H


struct fileman_config {
public:
	std::string hostname;
	size_t port;
	std::string username;
	std::string password;
	std::string cache_dir;
};

#endif //CODEX_WORKER_FILEMAN_CONFIG_H

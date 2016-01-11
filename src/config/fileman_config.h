#ifndef CODEX_WORKER_FILEMAN_CONFIG_H
#define CODEX_WORKER_FILEMAN_CONFIG_H


/**
 * Struct which stores informations which are usefull in file managers
 */
struct fileman_config {
public:
	/**
	 * Remote address of file server.
	 */
	std::string remote_url = "";
	/**
	 * Port which is used for connection to file server.
	 */
	size_t port = 0;
	/**
	 * Classical credentials.
	 */
	std::string username = "";
	/**
	 * Classical credentials.
	 */
	std::string password = "";
	/**
	 * Directory in which files will be cached (worker side).
	 */
	std::string cache_dir = "";


	/**
	 * Classic equality operator. All variables should match.
	 * @param second
	 * @return true if this structure and second has same values in variables
	 */
	bool operator==(const fileman_config &second) const
	{
		return (remote_url == second.remote_url &&
				port == second.port &&
				username == second.username &&
				password == second.password &&
				cache_dir == second.cache_dir);
	}

	/**
	 * Opossite for equality operator.
	 * @param second
	 * @return true if structures has different variables
	 */
	bool operator!=(const fileman_config &second) const
	{
		return !((*this) == second);
	}
};

#endif //CODEX_WORKER_FILEMAN_CONFIG_H

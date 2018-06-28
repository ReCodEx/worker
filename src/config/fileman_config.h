#ifndef RECODEX_WORKER_FILEMAN_CONFIG_H
#define RECODEX_WORKER_FILEMAN_CONFIG_H

#include <string>

/**
 * Struct which stores informations which are usefull in file managers.
 */
struct fileman_config {
public:
	/** Remote address of file server, port can be specified as well. */
	std::string remote_url = "";
	/** Classical credentials. */
	std::string username = "";
	/** Classical credentials. */
	std::string password = "";

	/**
	 * Classic equality operator. All variables should match.
	 * @param second compared structure
	 * @return true if this structure and second has same values in variables
	 */
	bool operator==(const fileman_config &second) const
	{
		return (remote_url == second.remote_url && username == second.username && password == second.password);
	}

	/**
	 * Opossite for equality operator.
	 * @param second compared structure
	 * @return true if structures has different variables
	 */
	bool operator!=(const fileman_config &second) const
	{
		return !((*this) == second);
	}
};

#endif // RECODEX_WORKER_FILEMAN_CONFIG_H

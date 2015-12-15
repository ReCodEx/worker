
#ifndef CODEX_WORKER_FILE_MANAGER_BASE_H
#define CODEX_WORKER_FILE_MANAGER_BASE_H

#include <string>
#include <exception>


/**
 * Base class for file manager.
 * File manager can get you a copy of file to some directory or put a file somewhere.
 * Failed operations throws @a fm_exception exception.
 * For more info, see derived classes.
 */
class file_manager_base {
public:
	/**
	 * Destructor.
	 */
    virtual ~file_manager_base() {}
    /**
     * Get the file.
     * @param src_name Name of the file to retrieve. Mostly this is sha1sum of the file.
     * @param dst_path Path to directory, where the file will be copied. Name of the file
     *                 will not change.
     */
	virtual void get_file(const std::string &src_name, const std::string &dst_path) = 0;
    /**
     * Put the file.
     * @param name Name of the file, which should be put somewhere. Possible use cases are
     *             insert this file to cache or send this file to remote server.
     */
	virtual void put_file(const std::string &name) = 0;
	/**
	 * Set data at runtine.
	 * @param destination Address where to get files from. Depending of derived classes this could
	 *					  be a server URL or local directory for caching.
	 * @param username Username if authentication is required.
	 * @param password Password if authentication is required.
	 */
	virtual void set_params(const std::string &destination, const std::string &username, const std::string &password) = 0;
	/**
	 * Get already set destination string.
	 * @return destination
	 */
	virtual std::string get_destination() const = 0;
};



/**
 * Generic file manager exception.
 */
class fm_exception : public std::exception {
public:
	fm_exception() : what_{"Generic file manager exception"} {}
	fm_exception(std::string what) : what_{what} {}
	virtual ~fm_exception() {}
	virtual const char* what() const noexcept
	{
		return what_.c_str();
	}
protected:
	std::string what_;
};

#endif //CODEX_WORKER_FILE_MANAGER_BASE_H

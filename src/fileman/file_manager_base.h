#ifndef RECODEX_WORKER_FILE_MANAGER_BASE_H
#define RECODEX_WORKER_FILE_MANAGER_BASE_H

#include <string>
#include <exception>


/**
 * Base class for file manager.
 * File manager can get you a copy of file to some directory or put a file somewhere.
 * Failed operations throws @a fm_exception exception.
 * For more info, see derived classes.
 */
class file_manager_base
{
public:
	/**
	 * Destructor.
	 */
	virtual ~file_manager_base()
	{
	}
	/**
	 * Get the file.
	 * @param src_name Name of the file to retrieve. Mostly this is sha1sum of the file.
	 * @param dst_name Path to file, where the data will be copied.
	 */
	virtual void get_file(const std::string &src_name, const std::string &dst_name) = 0;
	/**
	 * Put the file.
	 * @param src_name Name of the file, which should be put somewhere. Possible use cases are
	 *                 insert this file to cache or send this file to remote server.
	 * @param dst_path Where the file should be stored.
	 */
	virtual void put_file(const std::string &src_name, const std::string &dst_path) = 0;
};


/**
 * Generic file manager exception.
 */
class fm_exception : public std::exception
{
public:
	/**
	 * Constructor with default string.
	 */
	fm_exception() : what_("Generic file manager exception")
	{
	}
	/**
	 * Constructor with custom string.
	 * @param what String with description of failure.
	 */
	fm_exception(std::string what) : what_(what)
	{
	}
	/**
	 * Destructor.
	 */
	virtual ~fm_exception()
	{
	}
	/**
	 * Get failure description.
	 * @return Stored string.
	 */
	virtual const char *what() const noexcept
	{
		return what_.c_str();
	}

protected:
	/** Error message. */
	std::string what_;
};

#endif // RECODEX_WORKER_FILE_MANAGER_BASE_H

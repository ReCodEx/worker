#ifndef RECODEX_WORKER_PREFIXED_FILE_MANAGER_H
#define RECODEX_WORKER_PREFIXED_FILE_MANAGER_H

#include <memory>
#include "file_manager_interface.h"

/**
 * Prefixed file manager is just simple wrapper around any other file
 * manager, which adds configured prefix before source name in get_file()
 * method and before destination name in put_file() method.
 * Sample usage is set URI address of file server to manager and then provide
 * only filenames to method of http manager.
 */
class prefixed_file_manager : public file_manager_interface
{
private:
	/** String to prepend. */
	const std::string prefix_;
	/** Base file manager which need to be prefixed. */
	std::shared_ptr<file_manager_interface> fm_;

public:
	/**
	 * Constructor with initialization.
	 *
	 * @param fm Base file manager which need to be prefixed.
	 * @param prefix String to be prepend to source name in get_file()
	 *				 method and destination name in put_file() method.
	 */
	prefixed_file_manager(std::shared_ptr<file_manager_interface> fm, const std::string &prefix);

	/**
	 * Get file. This method has same semantics and arguments as underlying
	 * file manager, but @a src_name argument gets prefixed before calling
	 * base manager's get_file method.
	 *
	 * @param src_name Source file - same as underlying file manager
	 * @param dst_name Destination file - same as underlying file manager
	 */
	virtual void get_file(const std::string &src_name, const std::string &dst_name);

	/**
	 * Put file. This method has same semantics and arguments as underlying
	 * file manager, but @a dst_name argument gets prefixed before calling
	 * base manager's get_file method.
	 *
	 * @param src_name Source file - same as underlying file manager
	 * @param dst_name Destination file - same as underlying file manager
	 */
	virtual void put_file(const std::string &src_name, const std::string &dst_name);
};


#endif // RECODEX_WORKER_PREFIXED_FILE_MANAGER_H

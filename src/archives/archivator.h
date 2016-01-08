#ifndef CODEX_WORKER_ARCHIVE_MANAGER_H
#define CODEX_WORKER_ARCHIVE_MANAGER_H

#ifdef __linux__
	#define	_FILE_OFFSET_BITS 64
#endif
#include "archive.h"
#include "archive_entry.h"
#include <exception>
#include <string>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


class archivator {
public:
	static void compress(const std::string &dir, const std::string &destination);
	static void decompress(const std::string &filename, const std::string &destination);
private:
	static void copy_data(archive *ar, archive *aw);
};


/**
 * Generic archive manager exception.
 */
class archive_exception : public std::exception {
public:
	/**
	 * Constructor with default string.
	 */
	archive_exception() : what_("Generic file manager exception") {}
	/**
	 * Constructor with custom string.
	 * @param what String with description of failure.
	 */
	archive_exception(std::string what) : what_(what) {}
	/**
	 * Destructor.
	 */
	virtual ~archive_exception() {}
	/**
	 * Get failure description.
	 * @return Stored string.
	 */
	virtual const char* what() const noexcept
	{
		return what_.c_str();
	}
protected:
	std::string what_;
};

#endif //CODEX_WORKER_ARCHIVE_MANAGER_H

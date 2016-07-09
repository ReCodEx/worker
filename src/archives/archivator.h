#ifndef RECODEX_WORKER_ARCHIVE_MANAGER_H
#define RECODEX_WORKER_ARCHIVE_MANAGER_H

#ifdef __linux__
#define _FILE_OFFSET_BITS 64
#endif
#include "archive.h"
#include "archive_entry.h"
#include <exception>
#include <string>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

/**
 * Class for creating and decompressing archives.
 * On error, both methods throws @ref archive_exception.
 */
class archivator
{
public:
	/**
	 * This method will create new .zip archive containing recursively all files inside
	 * @a dir directory. The archive will contain one root directory (named as whole archive
	 * without extension). Example:
	 * @code{.cpp}
	 * archivator::compress("/home", "/tmp/homes.zip");
	 * @endcode
	 * Code above will compress @a /home directory to archive @a /tmp/homes.zip. The archive will
	 * contain one root directory @a homes, where are placed copies of all files and subdirectories
	 * in @a /home.
	 * @param dir Directory to compress.
	 * @param destination Name and path to the destination archive (should have suffix .zip).
	 * @throws archive_exception if any error occured
	 */
	static void compress(const std::string &dir, const std::string &destination);
	/**
	 * This method will decompress archive @a filename into directory @a destination.
	 * Supported formats are mainly zip, tar, tar.gz, tar.bz2, 7zip. Archive could contain
	 * only regular files or directories (ie. no symlinks, block and character devices,
	 * sockets or pipes allowed).
	 * For more info about supported formats see https://github.com/libarchive/libarchive/wiki.
	 * @note In source archive, paths containing ".." are not allowed!
	 * @param filename Archive to extract.
	 * @param destination Directory, where will be extracted files stored.
	 * @throws archive_exception if any error occured
	 */
	static void decompress(const std::string &filename, const std::string &destination);

private:
	/**
	 * Copy one entry from source archive @a ar to archive @a aw.
	 * @param ar source archive
	 * @param aw destination archive
	 */
	static void copy_data(archive *ar, archive *aw);
};


/**
 * Generic archive manager exception.
 */
class archive_exception : public std::exception
{
public:
	/**
	 * Constructor with default string.
	 */
	archive_exception() : what_("Generic file manager exception")
	{
	}
	/**
	 * Constructor with custom string.
	 * @param what String with description of failure.
	 */
	archive_exception(std::string what) : what_(what)
	{
	}

	/**
	 * Destructor.
	 */
	virtual ~archive_exception()
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
	/** Textual description of failure. */
	std::string what_;
};

#endif // RECODEX_WORKER_ARCHIVE_MANAGER_H

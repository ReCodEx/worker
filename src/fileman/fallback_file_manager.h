#ifndef CODEX_WORKER_FILE_MANAGER_H
#define CODEX_WORKER_FILE_MANAGER_H

#include "file_manager_base.h"
#include "cache_manager.h"
#include "http_manager.h"
#include <utility>


/**
 * Combines two file managers in a hierarchical manner.
 * When looking for a file, it tries the primary file manager first. If nothing is found, it tries the secondary one.
 * If that helps, it stores the file using the primary file manager.
 * Files are saved using the secondary manager.
 * Note that both managers need to use the same file names - some prefixing might need to be done
 * Failed operations throws @a fm_exception exception.
 */
class fallback_file_manager : public file_manager_base
{
public:
	typedef std::shared_ptr<file_manager_base> file_manager_ptr;

public:
	/**
	 * Constructor for creating this class with instances of @a cache_manager and @a http_manager.
	 * This is useful especially for testing with mocked classes.
	 * @param cache_manager Cache manager poiter.
	 * @param http_manager HTTP manager pointer.
	 */
	fallback_file_manager(file_manager_ptr primary, file_manager_ptr secondary);

	/**
	 * Destructor.
	 */
	virtual ~fallback_file_manager()
	{
	}

	/**
	 * Get file. If requested file is in cache, it'll be copied to @a dst_path immediately,
	 * otherwise it'll be downloaded to cache first.
	 * @param src_name Name of requested file.
	 * @param dst_path Path where to save the file.
	 */
	virtual void get_file(const std::string &src_name, const std::string &dst_path);

	/**
	 * Upload file to remote server. It won't be saved to cache.
	 * @param name Name of the file to upload.
	 */
	virtual void put_file(const std::string &src_name, const std::string &dst_path);

private:
	file_manager_ptr primary_manager_;
	file_manager_ptr secondary_manager_;
};

#endif // CODEX_WORKER_FILE_MANAGER_H

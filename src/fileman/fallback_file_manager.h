#ifndef RECODEX_WORKER_FILE_MANAGER_H
#define RECODEX_WORKER_FILE_MANAGER_H

#include "file_manager_base.h"
#include "cache_manager.h"
#include "http_manager.h"
#include <utility>


/**
 * Combines two file managers in a hierarchical manner.
 * When looking for a file, it tries the primary file manager first. If nothing is found,
 * it tries the secondary one. If that helps, it stores the file using the primary file
 * manager. Files are saved using the secondary manager. In easy language, first one is
 * cache and second one is http manager for example.
 * Failed operations throws @a fm_exception exception.
 * @note Both managers need to use the same file names - some prefixing might need to be
 *		 done (see @ref prefixed_file_manager).
 */
class fallback_file_manager : public file_manager_base
{
public:
	/** Pointer to every file manager type. */
	typedef std::shared_ptr<file_manager_base> file_manager_ptr;

public:
	/**
	 * Constructor for creating this class with two instances of file manager (our only case is
	 * @a cache_manager and @a http_manager). This is useful especially for testing with mocked classes.
	 * @param primary Primary manager (cache) instance poiter.
	 * @param secondary Secondary manager (HTTP) instance pointer.
	 */
	fallback_file_manager(file_manager_ptr primary, file_manager_ptr secondary);

	/**
	 * Destructor.
	 */
	virtual ~fallback_file_manager()
	{
	}

	/**
	 * Get file. If requested file is in cache, copy will be saved as @a dst_name immediately,
	 * otherwise it'll be downloaded to cache first and copied to requested destination later.
	 * @param src_name Name of requested file.
	 * @param dst_name Path (with filename) where to save the file (actual path you want,
	 *					caching is transparent from this point of view).
	 */
	virtual void get_file(const std::string &src_name, const std::string &dst_name);

	/**
	 * Save file using only secondary manager (i.e. upload file to remote server).
	 * It won't be saved to cache.
	 * @param src_name Name of the file (with path) to upload.
	 * @param dst_url Destinaton (url where to upload the file).
	 */
	virtual void put_file(const std::string &src_name, const std::string &dst_url);

private:
	/** Primary file manager (cache). */
	file_manager_ptr primary_manager_;
	/** Secondary file manager. */
	file_manager_ptr secondary_manager_;
};

#endif // RECODEX_WORKER_FILE_MANAGER_H

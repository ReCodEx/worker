#ifndef RECODEX_WORKER_INTERNAL_CP_DIR_TASK_H
#define RECODEX_WORKER_INTERNAL_CP_DIR_TASK_H

#include <boost/filesystem.hpp>
#include "../task_base.h"

namespace fs = boost::filesystem;

/**
 * Copy a directory recursively, with a limit on the total size of the output.
 * The files are taken in the order of size (ascending) and files that exceed the size limit are replaced with empty
 * files whose names are suffixed with ".skipped".
 */
class dump_dir_task: public task_base {
public:
	/**
	 * Constructor with initialization.
	 * @param id Unique identifier of load order of tasks.
	 * @param task_meta Variable containing further info about task. It's required that
	 * @a cmd_args entry has 2 or 3 arguments - the source, destination, and optionally a limit
	 * @throws task_exception on invalid number of arguments.
	 */
	dump_dir_task(size_t id, std::shared_ptr<task_metadata> task_meta);

	/**
	 * Destructor.
	 */
	virtual ~dump_dir_task();

	/**
	 * Run the action.
	 * @return Evaluation results to be pushed back to frontend.
	 */
	virtual std::shared_ptr<task_results> run();

private:
	bool copy_file(const fs::path &src, const fs::path &dest);
};


#endif //RECODEX_WORKER_INTERNAL_CP_DIR_TASK_H

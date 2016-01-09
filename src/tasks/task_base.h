#ifndef CODEX_WORKER_TASK_BASE_HPP
#define CODEX_WORKER_TASK_BASE_HPP

#include <vector>
#include <string>
#include <cstdlib>
#include <memory>


/**
 * Logical base of all possible tasks.
 * Its abstract class which cannot be instantiated.
 * But it stores some information about task which always have to be defined.
 */
class task_base {
public:
	/**
	 * Default constructor deleted.
	 */
	task_base() = delete;
	/**
	 * Only possible way of construction which just store all given parameters into private variables.
	 * @param id
	 * @param task_id
	 * @param priority
	 * @param fatal
	 * @param dependencies
	 * @param cmd
	 * @param arguments
	 * @param log
	 */
	task_base(size_t id, std::string task_id, size_t priority, bool fatal,
			  const std::vector<std::string> &dependencies,
			  const std::string &cmd, const std::vector<std::string> &arguments,
			  const std::string &log);
	/**
	 * Virtual destructor.
	 */
	virtual ~task_base();

	/**
	 * This method runs operation which this task is supposed to do.
	 */
	virtual void run() = 0;
	/**
	 * Add child to this task. Once given child cannot be deleted.
	 * @param add
	 */
	void add_children(std::shared_ptr<task_base> add);
	/**
	 * Get all children of task.
	 * @return constant reference to vector of task_base pointers
	 */
	const std::vector<std::shared_ptr<task_base>> &get_children();
	/**
	 * To this task object add parent.
	 * @param add
	 */
	void add_parent(std::shared_ptr<task_base> add);

	/**
	 * Get unique identification number.
	 * This number expresses order in job configuration.
	 * @return unsigned integer
	 */
	size_t get_id();
	/**
	 * Unique task ID which was stated in job configuration.
	 * @return textual description of id
	 */
	const std::string &get_task_id();
	/**
	 * Get priority of this task.
	 * Lower number -> higher priority.
	 * @return unsigned integer
	 */
	size_t get_priority();
	/**
	 * If true than failure of this task will cause immediate exit of job evaluation.
	 * @return
	 */
	bool get_fatal_failure();
	/**
	 * Get command which will be executed within this task.
	 * @return textual description of command
	 */
	const std::string &get_cmd();
	/**
	 * Arguments which will be supplied to executed command.
	 * @return constant vector of texts
	 */
	const std::vector<std::string> &get_args();
	/**
	 * Get name of the log.
	 * @return text
	 */
	const std::string &get_log();
	/**
	 * Return dependencies of this particular task.
	 * @return textual vector array of dependencies
	 */
	const std::vector<std::string> &get_dependencies();

protected:
	size_t id_;
	std::string task_id_;
	size_t priority_;
	bool fatal_failure_;
	std::string cmd_;
	std::string log_;
	std::vector<std::string> dependencies_;
	std::vector<std::string> arguments_;

	std::vector<std::weak_ptr<task_base>> parents_;
	std::vector<std::shared_ptr<task_base>> children_;
};

class task_exception : public std::exception {
public:
	task_exception() : what_("Generic task exception") {}
	task_exception(const std::string &what) : what_(what) {}
	virtual ~task_exception() {}
	virtual const char* what() const noexcept
	{
		return what_.c_str();
	}
protected:
	std::string what_;
};

#endif //CODEX_WORKER_TASK_BASE_HPP

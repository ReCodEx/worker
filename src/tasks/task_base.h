#ifndef RECODEX_WORKER_TASK_BASE_HPP
#define RECODEX_WORKER_TASK_BASE_HPP

#include <vector>
#include <string>
#include <cstdlib>
#include <memory>
#include <sstream>
#include "../config/task_results.h"
#include "../config/task_metadata.h"


/**
 * Logical base of all possible tasks.
 * It's abstract class which cannot be instantiated. But it stores some information
 * about task which always have to be defined.
 * @note Task can be created only with one constructor which receive pointer to
 * @ref task_metadata as parameter. This structure is mutable, so it can be changed
 * during task execution. This case should never happen in ReCodEx worker, but posibility
 * is still here. Please keep this in mind if you're editing this code otherwise it can
 * end up very bad for you!
 */
class task_base
{
public:
	/**
	 * Default constructor is disabled.
	 */
	task_base() = delete;
	/**
	 * Only possible way of construction which just store all given parameters into private variables.
	 * @param id Unique identificator of load order of tasks.
	 * @param task_meta Variable containing further info about task.
	 */
	task_base(size_t id, std::shared_ptr<task_metadata> task_meta);
	/**
	 * Virtual destructor.
	 */
	virtual ~task_base();

	/**
	 * This method runs operation which this task is supposed to do.
	 * @return Evaluation results to be pushed back to frontend.
	 */
	virtual std::shared_ptr<task_results> run() = 0;
	/**
	 * Add child to this task. Once given, child cannot be deleted.
	 * @param add Pointer to child task (task dependent on current one).
	 */
	void add_children(std::shared_ptr<task_base> add);
	/**
	 * Get all children of task.
	 * @return List of children tasks.
	 */
	const std::vector<std::shared_ptr<task_base>> &get_children();
	/**
	 * To this task object add parent.
	 * @param add Parent of this task. Execution may proceed only if parent task is
	 * successfully finished.
	 */
	void add_parent(std::shared_ptr<task_base> add);
	/**
	 * Get all parents of task.
	 * @return List of parent tasks.
	 */
	const std::vector<std::weak_ptr<task_base>> &get_parents();

	/**
	 * Get unique identification number.
	 * This number expresses order in job configuration.
	 * @return Task's identification number.
	 */
	size_t get_id();
	/**
	 * Unique task ID which was stated in job configuration.
	 * @return Unique textual description of current task.
	 */
	const std::string &get_task_id();
	/**
	 * Get priority of this task.
	 * Lower number = higher priority.
	 * @return Priority.
	 */
	size_t get_priority();
	/**
	 * Get failing policy. If @a true than failure of this task will cause
	 * mmediate exit of job evaluation.
	 * @return If task's failure is fatal for whole job.
	 */
	bool get_fatal_failure();
	/**
	 * Get a (shell) command which will be executed within this task. Same value
	 * as @a binary item inside @ref task_metadata structure during class construction.
	 * @return Command binary.
	 */
	const std::string &get_cmd();
	/**
	 * Arguments which will be supplied to executed command.
	 * @return List of text arguments.
	 */
	const std::vector<std::string> &get_args();
	/**
	 * Return dependencies of this particular task.
	 * @return List of dependencies - textual IDs of predecessor tasks. Same value
	 * as @a dependencies item inside @ref task_metadata structure during class construction.
	 */
	const std::vector<std::string> &get_dependencies();
	/**
	 * Return type of this task.
	 * @return task_type enum with all possible types
	 */
	task_type get_type();

	/**
	 * Tells whether task can be safely executed or not (ie. if parent task is failed).
	 * @return @a true if task can be executed.
	 */
	bool is_executable();
	/**
	 * Set execution bit to this task.
	 * @param set @a false if task cannot be safely executed, @a true otherwise (default value).
	 */
	void set_execution(bool set);
	/**
	 * Calls @a set_execution function on all children of this task.
	 * @param set Flag which will be passed to children.
	 */
	void set_children_execution(bool set);

protected:
	/** Unique integer ID of task. */
	size_t id_;
	/** Information about this task loaded from configuration file. */
	std::shared_ptr<task_metadata> task_meta_;
	/** If true task can be executed safely, otherwise its not wise. */
	bool execute_;

	/** Weak pointers to parents of task. */
	std::vector<std::weak_ptr<task_base>> parents_;
	/** Pointers to task children. */
	std::vector<std::shared_ptr<task_base>> children_;
};


/**
 * Task exception class.
 */
class task_exception : public std::exception
{
public:
	/**
	 * Generic task exception with no specification.
	 */
	task_exception() : what_("Generic task exception")
	{
	}
	/**
	 * Exception with some brief description.
	 * @param what textual description of a problem
	 */
	task_exception(const std::string &what) : what_(what)
	{
	}
	/**
	 * Virtual destructor.
	 */
	virtual ~task_exception()
	{
	}
	/**
	 * Return description of this exception.
	 * @return Cause description as C string.
	 */
	virtual const char *what() const noexcept
	{
		return what_.c_str();
	}

protected:
	/** Error description. */
	std::string what_;
};

template <typename T>
T read_task_arg(const std::vector<std::string> &args, const size_t index, const T &default_value = T())
{
	if (index >= args.size()) { return default_value; }

	T value;
	std::stringstream ss(args.at(index));
	ss >> value;

	return value;
}

/**
 * Comparator which is used for topological sorting of tasks
 */
class task_compare
{
public:
	/**
	 * Compare @ref task_base objects by their priority and identifier. This is something like
	 * lesser than operator on @ref task_base objects.
	 * Its supposed that bigger number of priority is greater priority, so this tasks will be prefered.
	 * @param a First task to compare.
	 * @param b Second task to compare.
	 * @return @a true if parameter a is lesser than b
	 */
	bool operator()(std::shared_ptr<task_base> a, std::shared_ptr<task_base> b)
	{
		if (a->get_priority() > b->get_priority()) {
			return true;
		} else if (a->get_priority() == b->get_priority() && a->get_id() < b->get_id()) {
			return true;
		}

		return false;
	}
};

#endif // RECODEX_WORKER_TASK_BASE_HPP

#ifndef RECODEX_WORKER_HELPERS_TOPOLOGICAL_SORT_HPP
#define RECODEX_WORKER_HELPERS_TOPOLOGICAL_SORT_HPP

#include <map>
#include <queue>
#include <stack>
#include <set>
#include "../tasks/task_base.h"


namespace helpers
{
	/**
	 * Topological sort of tasks starting from root, priorities included.
	 * Result order is saved in result variable, which is cleared before computation.
	 * Its assumed that whole graph is reachable through root task.
	 * Priorities and configuration file order are taken into account.
	 * Bigger number of priority means greater priority and therefore appropriate task will be prefered.
	 * @param root base node from which sorting starts
	 * @param result queue of task in order of execution
	 * @throws top_sort_exception if cycle was detected
	 */
	void topological_sort(std::shared_ptr<task_base> root, std::vector<std::shared_ptr<task_base>> &result);


	/**
	 * Special exception for topological sort.
	 */
	class top_sort_exception : public std::exception
	{
	public:
		/**
		 * Generic constructor.
		 */
		top_sort_exception() : what_("Generic topological sort exception")
		{
		}
		/**
		 * Constructor with specified cause.
		 * @param what description of exception
		 */
		top_sort_exception(const std::string &what) : what_(what)
		{
		}

		/**
		 * Stated for completion.
		 */
		virtual ~top_sort_exception()
		{
		}

		/**
		 * Returns description of exception.
		 * @return c-style string
		 */
		virtual const char *what() const noexcept
		{
			return what_.c_str();
		}

	protected:
		/** Textual description of error. */
		std::string what_;
	};
} // namespace helpers

#endif // RECODEX_WORKER_HELPERS_TOPOLOGICAL_SORT_HPP

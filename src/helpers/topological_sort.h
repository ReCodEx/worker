#ifndef CODEX_WORKER_HELPERS_TOPOLOGICAL_SORT_HPP
#define CODEX_WORKER_HELPERS_TOPOLOGICAL_SORT_HPP

#include <map>
#include <queue>
#include <set>
#include "../tasks/task_base.h"


namespace helpers
{
/**
 * Topological sort of tasks starting from root.
 * Result order is saved in result variable, which is cleared before computation.
 * Priorities and configuration file order are taken into account.
 * @note Algorithm itself taken from: http://stackoverflow.com/a/11236027
 * @param root
 * @param effective_indegree
 * @param result
 * @throws top_sort_exception
 */
void topological_sort(std::shared_ptr<task_base> root,
	std::map<std::string, size_t> &effective_indegree,
	std::vector<std::shared_ptr<task_base>> &result);


/**
 * Special exception for topological sort
 */
class top_sort_exception : public std::exception
{
public:
	top_sort_exception() : what_("Generic topological sort exception")
	{
	}
	top_sort_exception(const std::string &what) : what_(what)
	{
	}
	virtual ~top_sort_exception()
	{
	}
	virtual const char *what() const noexcept
	{
		return what_.c_str();
	}

protected:
	std::string what_;
};
}

#endif // CODEX_WORKER_HELPERS_TOPOLOGICAL_SORT_HPP

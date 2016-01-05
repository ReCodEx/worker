#ifndef CODEX_WORKER_TASK_BASE_HPP
#define CODEX_WORKER_TASK_BASE_HPP

#include <vector>
#include <string>
#include <cstdlib>
#include <memory>


/**
 *
 */
class task_base {
public:
	task_base() = delete;
	task_base(size_t id, std::string task_id, size_t priority, bool fatal,
			  const std::vector<std::string> &dependencies,
			  const std::string &cmd, const std::vector<std::string> &arguments,
			  const std::string &log);
	virtual ~task_base();

	virtual void run() = 0;
	void add_children(std::shared_ptr<task_base> add);
	const std::vector<std::shared_ptr<task_base>> &get_children();
	void add_parent(std::shared_ptr<task_base> add);

	size_t get_id();
	const std::string &get_task_id();
	size_t get_priority();
	bool get_fatal_failure();
	const std::string &get_cmd();
	const std::string &get_log();
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

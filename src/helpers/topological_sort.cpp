#include "topological_sort.h"

typedef std::priority_queue<std::shared_ptr<task_base>, std::vector<std::shared_ptr<task_base>>, task_compare>
	priority_queue_type;

void helpers::topological_sort(std::shared_ptr<task_base> root, std::vector<std::shared_ptr<task_base>> &result)
{
	// clean queue of tasks if there are any elements
	result.clear();

	// first go through whole tree and insert it into priority queue
	std::set<std::string> visited;
	std::vector<std::shared_ptr<task_base>> search_stack;
	priority_queue_type prior_queue;

	search_stack.push_back(root);
	while (!search_stack.empty()) {
		std::shared_ptr<task_base> current = search_stack.back();
		search_stack.pop_back();

		// if visited, continue
		if (visited.find(current->get_task_id()) != visited.end()) { continue; }

		// not visited, insert into queue
		prior_queue.push(current);
		visited.insert(current->get_task_id());

		// go through children
		for (auto &child : current->get_children()) { search_stack.push_back(child); }
	}

	// fill search stack from previously created priority queue
	while (!prior_queue.empty()) {
		search_stack.push_back(prior_queue.top());
		prior_queue.pop();
	}

	// and now build queue of tasks
	visited.clear();
	std::set<std::string> processed;
	while (!search_stack.empty()) {
		auto current = search_stack.back();

		if (processed.find(current->get_task_id()) != processed.end()) {
			// already processed and inserted into result
			search_stack.pop_back();
			continue;
		}

		if (visited.find(current->get_task_id()) == visited.end()) {
			visited.insert(current->get_task_id());

			// not visited yet, make new priority queue of parents and fill it into stack
			priority_queue_type task_queue;
			for (auto &parent : current->get_parents()) { task_queue.push(parent.lock()); }
			while (!task_queue.empty()) {
				search_stack.push_back(task_queue.top());
				task_queue.pop();
			}

		} else {
			// visited, but not processed
			result.push_back(current);
			processed.insert(current->get_task_id());
			search_stack.pop_back();
		}
	}
}

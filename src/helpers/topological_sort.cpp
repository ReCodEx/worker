#include "topological_sort.h"

void helpers::topological_sort(std::shared_ptr<task_base> root, std::vector<std::shared_ptr<task_base>> &result)
{
	// clean queue of tasks if there are any elements
	result.clear();


	// first compute indegrees from whole graph using depth-first search with stack
	std::map<std::string, size_t> indegree;
	std::stack<std::shared_ptr<task_base>> search_stack;
	search_stack.push(root);
	indegree.insert({root->get_task_id(), 0});

	while (!search_stack.empty()) {
		std::shared_ptr<task_base> current = search_stack.top();
		search_stack.pop();

		for (auto &child : current->get_children()) {
			if (indegree.find(child->get_task_id()) == indegree.end()) {
				// child of current task was not visited yet, so make new entry in indegrees
				// also plan it for searching by inserting into search stack
				indegree.insert({child->get_task_id(), 1});
				search_stack.push(child);
			} else {
				// this child was visited and there is no need to search its subtree again
				// however indegree has to be incremented
				++indegree.at(child->get_task_id());
			}
		}
	}


	// and now build queue of tasks
	size_t visited = 0;
	std::priority_queue<std::shared_ptr<task_base>, std::vector<std::shared_ptr<task_base>>, task_compare> prior_queue;
	prior_queue.push(root);

	while (!prior_queue.empty()) {
		auto current = prior_queue.top();
		prior_queue.pop();
		visited++;

		result.push_back(current);

		for (auto &child : current->get_children()) {
			if (indegree.at(child->get_task_id()) > 0) {
				size_t decremented = --indegree.at(child->get_task_id());

				if (decremented == 0) {
					prior_queue.push(child);
				}
			} else {
				throw top_sort_exception("Cycle in tasks dependencies detected");
			}
		}
	}

	// In case we do not walk through whole tree, that means that there are cycles.
	if (visited != indegree.size()) {
		throw top_sort_exception("Cycle in tasks dependencies detected");
	}
}

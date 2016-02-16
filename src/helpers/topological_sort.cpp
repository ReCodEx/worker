#include "topological_sort.h"

void helpers::topological_sort(std::shared_ptr<task_base> root,
	std::map<std::string, size_t> &effective_indegree,
	std::vector<std::shared_ptr<task_base>> &result)
{
	// clean queue of tasks if there are any elements
	result.clear();

	std::priority_queue<std::shared_ptr<task_base>, std::vector<std::shared_ptr<task_base>>, task_compare> prior_queue;
	std::set<std::string> passed; // store tasks that were visited and queued

	prior_queue.push(root);
	passed.insert(root->get_task_id());

	while (!prior_queue.empty()) {
		auto top = prior_queue.top();
		prior_queue.pop();

		result.push_back(top);

		auto deps = top->get_children();
		for (auto &dep : deps) {
			if (effective_indegree.at(dep->get_task_id()) > 0) {
				size_t tmp = --effective_indegree.at(dep->get_task_id());

				if (tmp == 0) {
					prior_queue.push(dep);
					passed.insert(dep->get_task_id());
				}
			} else {
				throw top_sort_exception("Cycle in tasks dependencies detected");
			}
		}
	}

	// In case that we do not walk through whole tree.
	// Usually it means that there are cycles in tree.
	if (passed.size() != effective_indegree.size()) {
		throw top_sort_exception("Cycle in tasks dependencies detected");
	}

	return;
}

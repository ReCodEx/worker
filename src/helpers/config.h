#ifndef CODEX_WORKER_HELPERS_CONFIG_H
#define CODEX_WORKER_HELPERS_CONFIG_H

#include <memory>
#include <yaml-cpp/yaml.h>
#include "../config/job_metadata.h"
#include "../config/task_metadata.h"


namespace helpers
{
	/**
	 * From given configuration in yaml it build job_metadata structure and its tasks
	 * @param config
	 * @return
	 */
	std::shared_ptr<job_metadata> build_job_metadata(const YAML::Node &conf);

	/**
	 * Special exception for config helper functions/classes
	 */
	class config_exception : public std::exception {
	public:
		config_exception() : what_("Generic config exception") {}
		config_exception(const std::string &what) : what_(what) {}
		virtual ~config_exception() {}
		virtual const char* what() const noexcept
		{
			return what_.c_str();
		}
	protected:
		std::string what_;
	};
}


#endif //CODEX_WORKER_HELPERS_CONFIG_H

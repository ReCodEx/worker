#ifndef RECODEX_WORKER_HELPERS_CONFIG_H
#define RECODEX_WORKER_HELPERS_CONFIG_H

#include <memory>
#include <yaml-cpp/yaml.h>
#include "../config/job_metadata.h"
#include "../config/task_metadata.h"


namespace helpers
{
	/**
	 * From given configuration in yaml it build job_metadata structure and its tasks.
	 * @param conf YAML representation of job configuration
	 * @return pointer on job_metadata class
	 */
	std::shared_ptr<job_metadata> build_job_metadata(const YAML::Node &conf);

	/**
	 *
	 * @param type
	 * @return
	 */
	task_type get_task_type(const std::string &type);

	/**
	 * From given configuration in yaml get bind directories config for sandbox.
	 * @param lim YAML representation of bind directories element
	 * @return triplet of source, destination and mount point permissions
	 */
	std::vector<std::tuple<std::string, std::string, sandbox_limits::dir_perm>> get_bind_dirs(const YAML::Node &lim);


	/**
	 * Special exception for config helper functions/classes.
	 */
	class config_exception : public std::exception
	{
	public:
		/**
		 * Generic constructor.
		 */
		config_exception() : what_("Generic config exception")
		{
		}
		/**
		 * Constructor with specified cause.
		 * @param what cause of this exception
		 */
		config_exception(const std::string &what) : what_(what)
		{
		}

		/**
		 * Stated for completion.
		 */
		virtual ~config_exception()
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
}


#endif // RECODEX_WORKER_HELPERS_CONFIG_H

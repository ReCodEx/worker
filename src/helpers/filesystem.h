#ifndef RECODEX_WORKER_HELPERS_FILESYSTEM_HPP
#define RECODEX_WORKER_HELPERS_FILESYSTEM_HPP

#include "config/sandbox_limits.h"
#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


namespace helpers
{
	/**
	 * Recursively copy directory from source to destination.
	 * @param src source directory which content will be copied into @a dest
	 * @param dest destination path which should not exist
	 * @throws filesystem_exception with approprite description
	 */
	void copy_directory(const fs::path &src, const fs::path &dest);

	/**
	 * Normalize dots and double dots from given path.
	 * @param path path which will be processed
	 * @return path without dots and double dots
	 */
	fs::path normalize_path(const fs::path &path);

	/**
	 * Check if given path is relative and do not contain any ".." element.
	 * @param path path to be checked
	 * @return true if path is relative and without double dots
	 */
	bool check_relative(const fs::path &path);

	/**
	 * Find path outside sandbox on real filesystem, based on given path inside sandbox.
	 * @param inside path inside sandbox which will be resolved
	 * @param sandbox_chdir directory where sandbox is chdir-ed
	 * @param bound_dirs directories bound to sandbox
	 * @param source_dir source directory on local filesystem
	 * @return path outside sandbox
	 */
	fs::path find_path_outside_sandbox(const std::string &inside_path,
		const std::string &sandbox_chdir,
		std::vector<std::tuple<std::string, std::string, sandbox_limits::dir_perm>> &bound_dirs,
		const std::string &source_dir);


	/**
	 * Special exception for filesystem helper functions/classes.
	 */
	class filesystem_exception : public std::exception
	{
	public:
		/**
		 * Generic constructor.
		 */
		filesystem_exception() : what_("Generic filesystem exception")
		{
		}
		/**
		 * Constructor with specified cause.
		 * @param what cause of this exception
		 */
		filesystem_exception(const std::string &what) : what_(what)
		{
		}

		/**
		 * Stated for completion.
		 */
		~filesystem_exception() override = default;

		/**
		 * Returns description of exception.
		 * @return c-style string
		 */
		const char *what() const noexcept override
		{
			return what_.c_str();
		}

	protected:
		/** Textual description of error. */
		std::string what_;
	};
} // namespace helpers

#endif // RECODEX_WORKER_HELPERS_FILESYSTEM_HPP

#ifndef CODEX_WORKER_FILESYSTEM_HPP
#define CODEX_WORKER_FILESYSTEM_HPP

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


namespace helpers
{
	/**
	 * Recursively copy directory from source to destination.
	 * @param src source directory which content will be copied into @a dest
	 * @param dest destination should not exist
	 * @throws filesystem_exception with approprite description
	 */
	void copy_directory(const fs::path &src, const fs::path &dest);


	/**
	 * Special exception for filesystem helper functions/classes
	 */
	class filesystem_exception : public std::exception {
	public:
		filesystem_exception() : what_("Generic filesystem exception") {}
		filesystem_exception(const std::string &what) : what_(what) {}
		virtual ~filesystem_exception() {}
		virtual const char* what() const noexcept
		{
			return what_.c_str();
		}
	protected:
		std::string what_;
	};
}

#endif //CODEX_WORKER_FILESYSTEM_HPP

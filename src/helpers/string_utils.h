#ifndef RECODEX_WORKER_HELPERS_CONFIG_H
#define RECODEX_WORKER_HELPERS_CONFIG_H

#include <string>
#include <algorithm>

namespace helpers
{
	/**
	 * Generate random string of given length.
	 * @param length
	 * @return generated string
	 */
	std::string random_alphanum_string(std::size_t length);

	/**
	 * Filter non-printable characters from given string and write it back.
	 * @param text
	 */
	void filter_non_printable_chars(std::string &text);
} // namespace helpers


#endif // RECODEX_WORKER_HELPERS_CONFIG_H

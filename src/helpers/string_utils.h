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
	std::string random_alphanum_string(size_t length);
}


#endif // RECODEX_WORKER_HELPERS_CONFIG_H

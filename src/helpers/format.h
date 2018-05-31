#ifndef RECODEX_WORKER_HELPERS_FORMAT_H
#define RECODEX_WORKER_HELPERS_FORMAT_H

#include <sstream>

namespace helpers
{
	inline void format(std::ostringstream &)
	{
	}

	template <typename ArgT, typename... T> void format(std::ostringstream &oss, const ArgT &a, T... args)
	{
		oss << a;
		format(oss, args...);
	}
} // namespace helpers

#endif // RECODEX_WORKER_HELPERS_FORMAT_H

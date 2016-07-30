#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "worker_core.h"


/**
 * The entry point
 * @param argc number of CLI arguments
 * @param argv an array of CLI arguments
 * @return exit code
 */
int main(int argc, char **argv)
{
	std::vector<std::string> args(argv, argv + argc);
	try {
		worker_core core(args);
		core.run();
	} catch (...) {
		std::cerr << "Something very bad happened!" << std::endl;
		return 1;
	}

	return 0;
}

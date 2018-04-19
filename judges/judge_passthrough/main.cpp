/*
 * Passthrough judge which just outputs inputted file.
 * (C) 2018 ReCodEx Team <github.com/ReCodEx>
 *
 * Usage: recodex-judge-passthrough <file> <file>
 *
 * Exitcode:
 *  - 0: if output was provided
 *  - 2: errors were present during execution of comparision, error message should be visible on stderr
 *
 * Enjoy :o)
 */

#include <iostream>
#include <fstream>

#define	RES_OK		0
#define	RES_ERROR	2

using namespace std;


/*
 * Application entry point.
 */
int main(int argc, char **argv) {

	// Check amount of program arguments.
	if (argc != 3) {
		cerr << "Wrong amount of arguments." << endl;
		return RES_ERROR;
	}

	ifstream inputfile(argv[1]);
	cout << inputfile.rdbuf();
	inputfile.close();

	return RES_OK;
}

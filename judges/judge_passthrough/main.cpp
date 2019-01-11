/*
 * Passthrough judge is ment to be used with data-only-judge-wrapper.
 * It reads the first line of the input file where the expected exit code is,
 * passes the rest of the input file to the output, and yields given exit code.
 * 
 * (C) 2018 ReCodEx Team <github.com/ReCodEx>
 *
 * Usage: recodex-judge-passthrough <file> <file>
 *
 * Enjoy :o)
 */

#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>

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

	try {
		ifstream inputFile(argv[1]);
		
		// Get the first line which holds the exit code
		std::string firstLine;
		getline(inputFile, firstLine);
		int exitCode = stoi(firstLine);
		if (exitCode < 0 || exitCode > 255) {
			throw runtime_error("Invalid exit code value.");
		}

		// Copy the rest of the input file to the output...
		cout << inputFile.rdbuf();
		inputFile.close();

		return exitCode;
	}
	catch(exception &e) {
		cerr << "Internal Error: " << e.what() << endl;
		return RES_ERROR;
	}
}

/*
 * Judge for shuffled files.
 * (C) 2007 Martin Krulis <krulis@ksvi.mff.cuni.cz>
 * (consulted with Martin Mares <mj@ucw.cz>)
 * (C) 2016 ReCodEx Team <github.com/ReCodEx>
 *
 * This judge compares two text files and returns 0 if they matches (and 1 otherwise).
 * Two files are compared with no regards for whitespace (whitespace acts just like token delimiter).
 * Usage: shuffled [-[n][i][r]] <file1> <file2>
 *	-n	ignore newlines (newline is considered only a whitespace)
 *	-i	ignore items order on the row (tokens on each row may be permutated)
 *	-r	ignore order of rows (rows may be permutated); this option has no effect when "-n" is used
 *
 * Exitcode:
 *  - 0: files are same, percentage is given on stdout
 *  - 1: files are similar, percentage is given on stdout
 *  - 2: errors were present during execution of comparision, error message should be visible on stderr
 *
 * Enjoy :o)
 */
//#define DEBUG 

#include <vector>
#include <algorithm>
#include <set>

#include "token.h"

using namespace std;


/*
 * Small framework for switches.
 */

// Set of switches that were defined in program arguments.
typedef set<char> SWITCH_SET;
SWITCH_SET switches;

#define	HAS_SWITCH(SWITCH)	(switches.find(SWITCH) != switches.end())

#define	SWITCH_IGNORE_NEWLINES	'n'
#define	SWITCH_SHUFFLED_ROWS	'r'
#define	SWITCH_SHUFFLED_ITEMS	'i'


void loadSwitches(char *arg) {
	if ((strlen(arg) <= 1) || (*arg++ != '-'))
		error("Wrong argument \"%s\".", arg);

	while(*arg)
		switches.insert(*arg++);
}





// The data set (entire file is loaded here).
typedef vector<CRow> FILE_DATA;


#ifdef DEBUG
/*
 * Dump whole data set.
 */
void dump(FILE_DATA &data) {
	for(unsigned i = 0; i < data.size(); i++)
		data[i].dump();
}
#endif


/*
 * Loads data set from given token file. If newlines are ignored,
 * only one row will be created in the data set.
 */
void load(FILE_DATA &data, const char *fileName, bool ignoreNewlines) {
	
	// Open file.
	CFile file(fileName);
	file.ignoreNewlines = ignoreNewlines;
	
	// Initialize data.
	data.clear();
	
	// Read rows in cycle until whole file is read.
	while(!file.eof()) {
		
		// Create a new row.
		data.push_back( CRow() );

		// If there is a regular pattern in amount of tokens on rows, allocate exact capacity for the row.
		if ((data.size() > 1) && (data[data.size()-1].size() == data[data.size()-2].size()))
			data.back().reserve( data[data.size()-1].size() );

		// Load row of tokens.
		data.back().load(file);
		if (data.back().size() == 0)
			error("Error occured while reading file.");
	}
}


/*
 * Compares data sets and returns the RES_OK or RES_WRONG result..
 */
int compare(FILE_DATA &data1, FILE_DATA &data2) {
	if (data1.size() != data2.size())
		return RES_WRONG;

	for(unsigned int i = 0; i < data1.size(); i++)
		if (data1[i] != data2[i])
			return RES_WRONG;
		
	return RES_OK;
}





/*
 * Application entry point.
 */
int main(int argc, char **argv) {
	
	// Check amount of program arguments.
	if ((argc < 3) || (argc > 4))
		error("Wrong amount of arguments.");

	// Load switches.
	if (argc == 4) loadSwitches(argv[1]);

	// Initialize CRow shuffling (whether the row should sort items or not).
	CRow::shuffledItems = (HAS_SWITCH(SWITCH_SHUFFLED_ITEMS));

	// Load file data.
	FILE_DATA data1, data2;
	load(data1, argv[argc-2], HAS_SWITCH(SWITCH_IGNORE_NEWLINES));
	load(data2, argv[argc-1], HAS_SWITCH(SWITCH_IGNORE_NEWLINES));

	// Sort rows if necessary.
	if (HAS_SWITCH(SWITCH_SHUFFLED_ROWS)) {
		sort(data1.begin(), data1.end());
		sort(data2.begin(), data2.end());
	}

#ifdef DEBUG
	dump(data1);
	return 0;
#endif

	// Compare the data sets and write results.
	int res = compare(data1, data2);
	if (res == RES_OK) {
		printf("%lf", 1.0);
	} else {
		printf("%lf", 0.0);
	}

	return res;
}

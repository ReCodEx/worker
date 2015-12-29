/*
 * Comment filter for CodEx.
 * (C) 2007 Martin Mares <mj@ucw.cz>
 * (C) 2007 Martin Krulis <krulis@ksvi.mff.cuni.cz>
 * 
 * This application filters C-like comments from a text file.
 * The comment starts with double slash sequence (//) and finishes with newline.
 * If the comment takes whole line, then whole line is filtered.
 *
 * Usage: filter [inputFile [outputFile]]
 * If outputFile is ommited, std. output is used instead.
 * If both files are ommited, application uses std. input and output.
 *
 */

#include "io.h"




/*
 * Filters comments from sin stream and stores it into sout stream.
 */
void filterComment(STREAM sin, STREAM sout) {
	
	// Initialize vars.
	int ch = sgetc(sin), ch2;
	int newline = 1;

	while (1) {
		
		// Potentional comment begin.
		if (ch == '/') {
			
			// Check next character.
			ch2 = sgetc(sin);
			if (ch2 == '/') {

				// Comment started - skip all chars until end of line.
				ch = sgetc(sin);
				while ((ch >= 0) && (ch != '\n'))
					ch = sgetc(sin);

				// If comment spaned over whole line (ingore LF).
				if (newline) {
					ch = sgetc(sin);
					continue;
				}
			
			// Coment has not started.		
			} else {
				sputc(sout, ch);	// Print out slash.
				ch = ch2;
			}
		}
		
		// File ended or error occured during reading -> finish reading.
		if (ch < 0) break;
		
		// Print out character.
		sputc(sout, ch);

		// Check newline and get another character.
		newline = (ch == '\n');		
		ch = sgetc(sin);
	}
	
	// Check for errors.
	if (serror(sin))
		error("Error occured while reading input file.");
}





/*
 * Application entry point.
 */
int main(int argc, char **argv) {

	// Skip first parameter (program name).
	argv++;
	
	// Open input file.
	STREAM sin;
	if (argv) {
		if (!(sin = sopenRead(*argv)))
			error("Error: Input file \"%s\" can not be open.", *argv)
		argv++;
	} else
		sin = STREAM_STDIN;
	
	// Open output file.
	STREAM sout;
	if (argv) {
		if (!(sout = sopenWrite(*argv)))
			error("Error: Output file \"%s\" can not be open.", *argv);
	} else
		sout = STREAM_STDOUT;
	
	// Proceed with filtering.
	filterComment(sin, sout);
	
	sclose(sin);
	sclose(sout);
	return 0;
}

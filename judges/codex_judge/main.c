/*
 * Universal judge for CodEx.
 * (C) 2007 Martin Krulis <krulis@ksvi.mff.cuni.cz>
 *
 * This judge compares two text files. It compares only text tokens regardless amount of whitespace between them.
 * Usage: codex_judge [-r | -n | -rn] <file1> <file2>
 *	- file1 and file2 are paths to files that will be compared
 *	- switch options "r" and "n" can be specified as a 1st optional argument leading with "-".
 *		"n" - judge will treat newlines as ordinary whitespace (it will ignore line breaking)
 *		"r" - judge will treat tokens as real numbers and compares them accordingly (with some amount of error)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tokenize.h"


#define	TRUE		1
#define	FALSE		0

#define	RES_OK		0
#define	RES_WRONG	1
#define	RES_ERROR	2


/*
 * Margin of error is 0.001%. To avoid problems around zero, minimal absolute margin of error is 10^{-20}.
 */
#define	MARGIN_OF_ERROR		0.00001
#define	MINIMAL_EPSILON		1e-30


/*
 * Macro that writes out an error and exits the application with error code.
 */
#define	error(...)	{\
	fprintf(stderr, __VA_ARGS__);\
	exit(RES_ERROR);\
}


// Type of function that is used for comparing two tokens.
typedef int token_compare_function(TOKEN*, TOKEN*);


/*
 * Compare two tokens.
 * Tokens are converted into long doubles and then compared with given margin of error.
 */
int compareRealTokens(TOKEN *token1, TOKEN *token2) {
	
	// Convert tokens to doubles.
	long double x1, x2;
	int res1 = tokenToLongDouble(token1, &x1);
	int res2 = tokenToLongDouble(token2, &x2);
	
	// One token is double the other one is not -> they can't match.
	if (res1 != res2) return FALSE;
	
	// Tokens are not double -> comare them as strings.
	if (res1 != TOK_OK)
		return tokensEqual(token1, token2);
	
	// Compare doubles with given margins of errors.
	if (x1 == x2) return TRUE;

	long double epsilon = fabs(x1 * MARGIN_OF_ERROR);
	if (epsilon < MINIMAL_EPSILON) epsilon = MINIMAL_EPSILON;
	
	return (fabs(x1 - x2) <= epsilon);
}



/*
 * Function that compares two files. It returns RES_xxx values.
 */
int compare(TOKEN_FILE *f1, TOKEN_FILE *f2, token_compare_function *comp, int ignoreNewline) {
	
	// Initialize token structures.
	TOKEN token1, token2;
	initializeToken(&token1, 31);
	initializeToken(&token2, 31);
	
	// Read tokens in cycle.
	int res1 = fgetToken(f1, &token1);
	int res2 = fgetToken(f2, &token2);
	while((res1 == TOK_OK) && (res2 == TOK_OK)) {

		if (!(*comp)(&token1, &token2))
			break;
			
		if (!ignoreNewline && (token1.newline != token2.newline))
			break;
			
		// Get another tokens.
		res1 = fgetToken(f1, &token1);
		res2 = fgetToken(f2, &token2);
	}
	
	// Clean up token structures.
	freeToken(&token1);
	freeToken(&token2);
	
	return ((res1 == TOK_EOF) && (res2 == TOK_EOF)) ? RES_OK : RES_WRONG;
}



/*
 * Application entry point.
 */
int main(int argc, char **argv) {
	
	// Process parameters.
	TOKEN_FILE *f1, *f2;
	int realTokens = 0;
	int ignoreNewline = 0;
	
	if ((argc == 3) || (argc == 4)) {

		// Check for switch parameters.
		if (argc == 4) {
			if (argv[1][0] != '-')
				error("Invalid argument format \"%s\"", argv[1]);

			// For each switch char...
			int len = strlen(argv[1]);
			while(--len > 0)
				switch(argv[1][len]) {
					case 'n':	ignoreNewline = TRUE;	break;
					case 'r':	realTokens = TRUE;		break;
					default:
						error("Invalid argument format \"%s\"", argv[1]);
				}
		}

		// Open file 1.
		if (!(f1 = tfopen(argv[argc-2])))
			error("Can not open file \"%s\"", argv[argc-2]);

		// Open file 2.
		if (!(f2 = tfopen(argv[argc-1])))
			error("Can not open file \"%s\"", argv[argc-1]);
			
	} else
		error("Wrong amount of arguments.");
		
	
	// Proceed with comparation.
	token_compare_function *comp = tokensEqual;
	if (realTokens)
		comp = compareRealTokens;
	int res = compare(f1, f2, comp, ignoreNewline);

	// Close files.
	tfclose(f1);
	tfclose(f2);

    return res;
}

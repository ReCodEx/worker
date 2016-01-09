/*
 * Tokenize library (v 1.0.0).
 * (C) Martin Krulis <krulis@ksvi.mff.cuni.cz>, 2007
 */

#ifndef TOKENIZE_H_INCLUDED
#define TOKENIZE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//Disable warning about fopen() on Windows
#ifdef _WIN32
	#pragma warning(disable : 4996)
#endif

#define	_ISOC99_SOURCE
#include <stdio.h>
#include <stdlib.h>


/*
 * Maximal token capacity is defined 32 MBytes.
 */
#define	MAX_TOKEN_CAPACITY	(32 << 20)

/*
 * Ok results.
 */
#define	TOK_OK				0

/*
 * Error results (all error results are < 0).
 */
#define TOK_OUT_OF_MEMORY	-1
#define	TOK_INVALID_PARAMS	-2
#define TOK_EOF				-3
#define	TOK_READING_ERROR	-4
#define	TOK_INVALID_FORMAT	-5




/*
 * The token file - buffered standard file.
 */
#define	TOKEN_FILE_BUF_SIZE		65536

struct s_token_file;
#define	TOKEN_FILE	struct s_token_file


TOKEN_FILE *tfopen(const char *fileName);
void tfclose(TOKEN_FILE *tf);





/*
 * Token structure.
 * It keeps other information about token string to simplify realocation.
 */
struct s_token {
	char *str;
	int len, capacity, newline;
};
#define	TOKEN	struct s_token


int initializeToken(TOKEN *token, int capacity);
int fgetToken(TOKEN_FILE *tf, TOKEN *token);
int tokensEqual(TOKEN *token1, TOKEN *token2);
void freeToken(TOKEN *token);

int tokenToDouble(TOKEN *token, double *x);
int tokenToLongDouble(TOKEN *token, long double *x);
int tokenToLong(TOKEN *token, long *x);
int tokenToULong(TOKEN *token, unsigned long *x);



#ifdef __cplusplus
}
#endif
 
#endif // TOKENIZE_H_INCLUDED

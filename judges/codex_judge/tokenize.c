/*
 * Tokenize library (v 1.0.0).
 * (C) Martin Krulis <krulis@ksvi.mff.cuni.cz>, 2007
 *
 * This library is designed to streamline reading of text files without whitespace.
 * It also defines TOKEN_FILE files which implements simple buffering over stdio files.
 * For more information see attached documentation.
 *
 */

#include "tokenize.h"
#include <string.h>


/*
 * Inline functions for this file.
 */

/*
 * Returns true if ch is a whitespace character.
 */
inline int is_whitespace(char ch) {
	return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n');
}





/*
 * Internal control structure for token file.
 */
struct s_token_file {
	FILE *fp;		// Associated file (it is accessed via std. IO functions).
	char *buf;		// Reading buffer.
	int len, pos;	// Length of data in buffer and actual readin position.
};



/*
 * Inline local functions for token file.
 */
 
/*
 * Returns another character from the stream, but the char is not removed from the stream.
 * Returns EOF if the file has finished or error occured.
 */
inline int tfpeek(TOKEN_FILE *tf) {

	// If there are no more valid chars in the buffer.
	if (tf->len == tf->pos) {
		if (feof(tf->fp)) return EOF;
		tf->pos = 0;
		tf->len = fread(tf->buf, 1, TOKEN_FILE_BUF_SIZE, tf->fp);
		if (!tf->len) return EOF;
	}
	
	// Return char at the actual position in the buffer;
	return tf->buf[tf->pos];
}



/*
 * Returns another character from the stream.
 * Returns EOF if the file has finished or error occured.
 */
inline int tfgetc(TOKEN_FILE *tf) {
	int res = tfpeek(tf);
	tf->pos++;
	return res;
}



/*
 * Returns nonzero value if all data have been read.
 */
inline int tfeof(TOKEN_FILE *tf) {
	return (tf->len == tf->pos) && feof(tf->fp);
}



/*
 * Returns the same value as ferror on a regular file.
 */
inline int tferror(TOKEN_FILE *tf) {
	return ferror(tf->fp);
}





/*
 * Public functions for token file.
 */
 
/*
 * Open a token file and return's token file control structure.
 * If no file name is given (null is passed), token file will use stdin instead.
 * File is always opened as a binary file for reading.
 * Return's null if file can not be opened.
 */
TOKEN_FILE *tfopen(const char *fileName) {
	
	// Allocate token file control structure.
	TOKEN_FILE *tf = (TOKEN_FILE*)malloc( sizeof(TOKEN_FILE) );
	if (!tf) return NULL;
	
	// Open file.
	tf->fp = (fileName) ? fopen(fileName, "rb") : stdin;
	if (!tf->fp) goto error;
	
	// Allocate file buffer.
	tf->buf = (char*)malloc(TOKEN_FILE_BUF_SIZE);
	if (!tf->buf) goto error2;
	
	// Fill in other variables.
	tf->len = tf->pos = 0;
	
	// Skip leading whitespace.
	while(!tfeof(tf) && is_whitespace(tfpeek(tf)))
		tfgetc(tf);
	
	return tf;

error2:
	fclose(tf->fp);
error:
	free(tf);
	return NULL;
}



/*
 * Release all resources associated with given token file.
 */
void tfclose(TOKEN_FILE *tf) {
	fclose(tf->fp);
	free(tf->buf);
	free(tf);
}





/*
 * Token processing functions.
 */

/*
 * Initialize the token structure with given capacity.
 * Recomended initial capacity is 15 (or some 2^k - 1 number).
 */
int initializeToken(TOKEN *token, int capacity) {
	
	// Check params.
	if (!token || (capacity < 0))
		return TOK_INVALID_PARAMS;

	if (capacity > MAX_TOKEN_CAPACITY)
		return TOK_OUT_OF_MEMORY;
	
	// Allocate string.
	token->capacity = capacity;
	token->str = (char*)malloc(capacity+1);
	if (!token->str)
		return TOK_OUT_OF_MEMORY;
	
	// Set token to an empty string.
	token->len = 0;
	token->str[0] = '\0';
	
	// Set up other members.
	token->newline = 0;
	
	return TOK_OK;
}



/*
 * Private method that reallocates the token for bigger capacity.
 */
int reallocToken(TOKEN *token) {

	// Too small capacity -> make it at least 15.
	if (token->capacity < 15)
		token->capacity = 15;
	
	// Normal capacity -> double the size.
	else
		token->capacity += token->capacity + 1;
	
	if (token->capacity > MAX_TOKEN_CAPACITY)
		return TOK_OUT_OF_MEMORY;
	
	// Reallocate the token string.
	token->str = (char*)realloc(token->str, token->capacity + 1);
	if (!token->str) {
		token->len = token->capacity = 0;
		return TOK_OUT_OF_MEMORY;
	}
	
	return TOK_OK;
}



/*
 * Reads a token from opened token file. The token must be initialized.
 * If token was found function returns TOK_OK and valid data are in the token structure.
 * Otherwise an error occured and there may be anything in the token structure.
 */
int fgetToken(TOKEN_FILE *tf, TOKEN *token) {

	// Check params.
	if (!tf || !token)
		return TOK_INVALID_PARAMS;
	
	// Set token to empty string.
	token->len = 0;
	token->str[0] = '\0';

	// Skip leading whitespace.
	int c = tfpeek(tf);
	token->newline = 0;
	while ((c != EOF) && is_whitespace(c)) {
		if (c == '\n') token->newline = 1;
		tfgetc(tf);
		c = tfpeek(tf);
	}
		
	// If there are no more data or error occured, announce it.
	if (c == EOF) return (tferror(tf) == 0) ? TOK_EOF : TOK_READING_ERROR;
	
	// Read the token.
	while ((c != EOF) && !is_whitespace(c)) {
		
		// Ensure that token string capacity is sufficient.
		if (token->len == token->capacity) {
			if (reallocToken(token) != TOK_OK)
				return TOK_OUT_OF_MEMORY;
		}
		
		// Add char to the token and read another one.
		token->str[ token->len++ ] = tfgetc(tf);
		c = tfpeek(tf);
	}
	
	// Add trailing null char into token string.
	token->str[token->len] = '\0';
	
	return (tferror(tf) == 0) ? TOK_OK : TOK_READING_ERROR;
}



/*
 * Returns nonzero value if token1 equals token2 and zero otherwise.
 */
int tokensEqual(TOKEN *token1, TOKEN *token2) {

	// Check params.
	if (!token1 || !token2)
		return TOK_INVALID_PARAMS;
		
	if (token1->len != token2->len)
		return 0;
	
	return !strcmp(token1->str, token2->str);
}



/*
 * Release token string inside token structure.
 */
void freeToken(TOKEN *token) {
	if (token) free(token->str);
}





/*
 * Conversion functions. Each function tries to convert token into specific data type.
 * If the conversion succeeds TOK_OK is returned, otherwise TOK_INVALID_PARAMS is returned.
 */

#define	testInputParams	\
	if (!token || !x)\
		return TOK_INVALID_PARAMS;

	
#define tokenToXPrototype(FUNC, ...)	\
	testInputParams\
	char *end;\
	*x = FUNC(token->str, &end, ##__VA_ARGS__);\
	return (end == token->str + token->len) ? TOK_OK : TOK_INVALID_FORMAT;\



int tokenToDouble(TOKEN *token, double *x) {
	tokenToXPrototype(strtod)
}


int tokenToLongDouble(TOKEN *token, long double *x) {
	tokenToXPrototype(strtold)
}


int tokenToLong(TOKEN *token, long *x) {
	tokenToXPrototype(strtol, 10)
}


int tokenToULong(TOKEN *token, unsigned long *x) {
	testInputParams
	if (token->str[0] == '-')	// Unsigned number must be really "unsigned".
		return 0;
	tokenToXPrototype(strtoul, 10)
}

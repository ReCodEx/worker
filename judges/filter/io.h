/*
 * Simple buffered IO library.
 * (C) 2007 Martin Krulis <krulis@ksvi.mff.cuni.cz>
 */
#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>


/*
 * Prints an error mesage to stderr and return error code 1.
 */
#define	error(...) {\
	fprintf(stderr, __VA_ARGS__);\
	exit(1);\
}


/*
 * Stream abstraction. It is used so buffering can be implemented over libc or unix IO.
 */

struct s_stream;

// Stream is file handle.
#define	STREAM			struct s_stream *

// Representation of stdin and stdout as streams
#define	STREAM_STDIN	sopenRead(NULL)
#define	STREAM_STDOUT	sopenWrite(NULL)


STREAM sopenRead(const char *fileName);
STREAM sopenWrite(const char *fileName);
void sclose(STREAM s);

int sgetc(STREAM s);
void sputc(STREAM s, int ch);
int serror(STREAM s);


#endif // IO_H_INCLUDED

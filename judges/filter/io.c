/*
 * Simple buffered IO library.
 * (C) 2007 Martin Krulis <krulis@ksvi.mff.cuni.cz>
 */
#include "io.h"


// Length of the IO buffer.
#define BUF_LEN		65536


/*
 * Internal stream structure;
 */
struct s_stream {
	FILE *fp;
	char *buf;
	int len, pos;
};


/*
 * Private function that allocates STREAM structure for given file.
 */
STREAM createStream(FILE *fp) {
	STREAM res = (STREAM)malloc( sizeof(struct s_stream) );
	if (!res)
		error("Out of memory.");
	
	res->buf = (char*)malloc(BUF_LEN);
	if (!res->buf)
		error("Out of memory.");		

	res->fp = fp;
	res->len = res->pos = 0;
	return res;
}


/*
 * Private function that opens given file and create a structure for it.
 */
STREAM sopen(const char *fileName, const char *mode) {
	FILE *fp = fopen(fileName, mode);
	if (!fp)
		error("File \"%s\" can not be opened.", fileName);
	
	STREAM res = createStream(fp);
	if (!res)
		fclose(fp);
	
	return res;
}




/*
 * Opean given file as stream for reading.
 */
STREAM sopenRead(const char *fileName) {
	if (fileName)
		return sopen(fileName, "rb");
	else
		return createStream(stdin);
}


/*
 * Open given file as stream for writing.
 */
STREAM sopenWrite(const char *fileName) {
	STREAM res;
	if (fileName) {
		res = sopen(fileName, "wb");
	} else
		res = createStream(stdout);
	res->len = -1;
	return res;
}


/*
 * Closes given stream.
 */
void sclose(STREAM s) {
	if (s->len < 0) {
		fwrite(s->buf, 1, s->pos, s->fp);
	}
	free(s->buf);
	fclose(s->fp);
	free(s);
}


/*
 * Fetch one character from a stream. Negative value is returned on eof or error.
 */
inline int sgetc(STREAM s) {
	if (s->pos > s->len)
		error("Can not read from write-only stream.");

	// Refill buffer if necessary.
	if (s->pos == s->len) {
		s->pos = 0;
		s->len = fread(s->buf, 1, BUF_LEN, s->fp);
		if (ferror(s->fp))
			error("Error reading from file.");
		if (s->len == 0) return -1;
	}
	
	return s->buf[ s->pos++ ];
}



/*
 * Write one character into a stream.
 */
inline void sputc(STREAM s, int ch) {
	if (s->len != -1)
		error("Can not write into read-only stream.");

	if (s->pos == BUF_LEN) {
		fwrite(s->buf, 1, s->pos, s->fp);
		if (ferror(s->fp))
			error("Error writing into file.");
		s->pos = 0;
	}
	s->buf[ s->pos++ ] = ch;
}


/*
 * Check given stream for errors. Nonzero value is returned if any error occured in the stream.
 */
inline int serror(STREAM s) {
	return ferror(s->fp);
}

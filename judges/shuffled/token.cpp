/*
 * Token library for Shuffled judge.
 * (C) 2007 Martin Krulis <krulis@ksvi.mff.cuni.cz>
 *
 * Token lib implements a few classes that are used for tokenizing and token ordering a text file. 
 *
 */
#include "token.h"
#include <algorithm>


using namespace std;


/*
 * CFile methods implementation.
 */

/*
 * File reading abstraction. Reads entire file into one string.
 * Buffer is allocated as big as needed (plus trailing zero) and file length is returned.
 */
CFile::CFile(const char *fileName) {

	// Open the file.
	FILE *fp = fopen(fileName, "rb");
	if (!fp)
		error("File \"%s\" can not be open.", fileName);
	
	// Get file length.
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	if (len > MAX_FILE_SIZE)
		error("File \"%s\" is too big (%lu B).", fileName, len);
	fseek(fp, 0, SEEK_SET);
	
	// Allocate buffer.
	buf = pos = (char*)malloc(len+1);
	if (!buf)
		error("Out of memory.");
	
	// Read data (and close the file since it won't be needed any more).
	int count = fread(buf, 1, len, fp);
	if (count < len)
		error("Unexpected end of file \"%s\" (only %d from %lu bytes were read) [ferror = %d]", fileName, count, len, ferror(fp));
	fclose(fp);

	// Find end of the file and put a traling zero there.
	bufEnd = buf + count;
	*bufEnd = '\0';
	
	// Skip leading whitespace.
	skipWhitespace();
}


/*
 * Skip all whitespaces in buffer (and move buffer position). Function returns number of newlines encountered.
 */
inline int CFile::skipWhitespace() {
	int newlines = 0;

	// While there's a whitespace on actual position.
	while(!eof() && isWhitespace(getChar())) {
		
		// Check for newlines.
		if (getChar() == '\n')
			newlines++;

		// Convert whitespace into zero and move further.
		putZero();;
	}
	
	return ignoreNewlines ? 0 : newlines;
}





/*
 * CToken methods implementation.
 */

/*
 * Adds new char into hash code (for calculating hash value) and returns new hash.
 */
inline unsigned int CToken::addHashChar(unsigned int hash, char ch) {
	unsigned int x = (hash >> 20) & (1 << 5 - 1);
	hash = (hash << 5) ^ (unsigned)ch;
	hash = hash ^ x;
	return hash;
}


/*
 * Parse file and fill new token data into CToken object.
 */
bool CToken::parse(CFile &file) {

	// Clean token members.
	str = NULL;
	hash = 0;

	// Skip leading whitespace.
	int newline = file.skipWhitespace();
	
	// If newline was found of input ends - return empty token.
	if (newline || file.eof())
		return false;
			
	// Load token itself.
	str = file.getPos();
	unsigned int len = 0;
	while(!file.eof() && !file.isWhitespace()) {
		hash = addHashChar(hash, file.getChar());
		file.incPos();
		len++;
	}
	
	hash <<= 7;
	hash ^= len;
	return true;
}





/*
 * CRow methods implementation.
 */

// Static flag.
bool CRow::shuffledItems = false;


/*
 * Load row of tokens from given file.
 */
void CRow::load(CFile &file) {
	
	// Prepare variables.
	CToken token;
	hash = 0;

	// Read all tokens on the row.
	while( token.parse(file) ) {

		// Modify hash.
		if (!shuffledItems) {
			// If the items are not shuffled the hash should reflect ordering.
			unsigned int x = hash >> 27;
			hash = hash << 5;
			hash ^= token.getHash() ^ x;
		} else
			hash = (unsigned)(((unsigned long)hash + (unsigned long)token.getHash()) & 0xffffffff);

		// Store token into the vector and fetch another one.
		tokens.push_back(token);
	}
	
	// If items are shuffled, sort them.
	if (shuffledItems)
		sort(tokens.begin(), tokens.end());
}

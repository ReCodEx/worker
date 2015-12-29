/*
 * Token library for Shuffled judge.
 * (C) 2007 Martin Krulis <krulis@ksvi.mff.cuni.cz>
 */
#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED


#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>

#ifdef DEBUG
	#include <cstdio>
#endif


/*
 * Common return codes for all judges.
 */
#define	RES_OK		0
#define	RES_WRONG	1
#define	RES_ERROR	2


/*
 * Macro that writes out an error and exits the application with error code.
 */
#define	error(...)	{\
	fprintf(stderr, __VA_ARGS__);\
	exit(RES_ERROR);\
}


// Maximal file size that is allowed to be processe by this lib.
#define	MAX_FILE_SIZE	(32 << 20)





/*
 * Class that encapsulates file operations.
 * Whole file is read into one large buffer and all operations are performed on that buffer.
 */
class CFile {
private:
	char *buf;		// Allocated buffer for the file's data.
	char *pos;		// Actual position in the buffer.
	char *bufEnd;	// Pointer at the end of the buffer.

	
public:
	// Flag that indicates whether 
	bool ignoreNewlines;
	
	// Constructor - reads given file.
	CFile(const char *fileName);
	
	/*
	 * Inline functions.
	 */
	char getChar() const			{ return *pos; }
	void incPos()					{ pos++; }
	void putZero()					{ *pos++ = '\0'; }
	const char *getPos() const		{ return pos; }
	const char *getBufEnd() const	{ return bufEnd; }
	bool eof() const				{ return (pos == bufEnd); }
	
	static inline bool isWhitespace(char ch) {
		return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n');
	}
	inline bool isWhitespace()		{ return isWhitespace(getChar()); }
	inline int skipWhitespace();
};



/*
 * CToken encapsulates one token in the file stream.
 * It contains pointer to begining of the token and it's hash code.
 */
class CToken {
private:
	const char *str;	// Begining of the token in the file's buffer.
	unsigned int hash;	// XOR hash code of the token.

	static inline unsigned int addHashChar(unsigned int hash, char ch);

	
public:
	unsigned int getHash()			{ return hash; }
	bool parse(CFile &file);

#ifdef DEBUG
	void dump()	{ printf("Token [%x]: \"%s\"\n", hash, str); }
#endif

	// Comparing function and operators override.
	int compare(CToken const &token) const {
		if (hash == token.hash) {
			return strcmp(str, token.str);
		} else
			return (hash < token.hash) ? -1 : 1;
		
	}
	
	bool operator !=(const CToken &token) const {
		return (compare(token) != 0);
	}
	
	bool operator <(const CToken &token) const {
		return (compare(token) < 0);
	}

};



/*
 * CRow encapsulates one row of tokens. 
 */
class CRow {
private:
	std::vector<CToken> tokens;		// Vector of token objects on the row.
	unsigned int hash;				// XOR hash code of the entire row (it is calculated from hashes of all tokens).
	
public:
	// Static flag for all rows - whether the items are shuffled.
	static bool shuffledItems;
	
	unsigned size()				{ return tokens.size(); }
	void reserve(int capacity)	{ tokens.reserve(capacity); }
	
	void load(CFile &file);
//	void sortTokens();

#ifdef DEBUG
	void dump()	{
		printf("Row [%x] with %d tokens:\n", hash, size());
		for(unsigned i = 0; i < size(); i++)
			tokens[i].dump();
		printf("\n");
	}
#endif

	// Operator overrides.
	bool operator ==(const CRow &row) const {
		// Quick check - hashes and lengths.
		if ((hash != row.hash) || tokens.size() != row.tokens.size())
			return false;
		
		// Check every token on the row.
		for(unsigned i = 0; i < tokens.size(); i++)
			if (tokens[i] != row.tokens[i]) return false;

		return true;
	}
	
	bool operator !=(const CRow &row) const {
		return !(*this == row);
	}
	
	bool operator <(const CRow &row) const {
		if (hash == row.hash) {
			if (tokens.size() == row.tokens.size()) {
				// Check all tokens.
				for(unsigned i = 0; i < tokens.size(); i++) {
					int res = tokens[i].compare(row.tokens[i]);
					if (res < 0) return true;
					if (res > 0) return false;
				}
				
				// All tokens are equal;
				return false;
			} else
				return (tokens.size() < row.tokens.size());
		} else
			return (hash < row.hash);
	}
};



#endif // TOKEN_H_INCLUDED

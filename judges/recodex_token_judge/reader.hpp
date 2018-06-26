#ifndef RECODEX_TOKEN_JUDGE_READER_HPP
#define RECODEX_TOKEN_JUDGE_READER_HPP


#include <system/mmap_file.hpp>
#include <misc/ptr_fix.hpp>

#include <memory>
#include <string>
#include <vector>
#include <limits>
#include <cstdint>
#include <cctype>
#include <cstddef>


/**
 * Reader is a wrapper that mmaps file for reading and provide parsing function.
 * \tparam CHAR Base character type used for parsing (char by default).
 * \tparam OFFSET Base data type for numeric offsets.
 *         The offset determines maximal file size that can be processed.
 */
template <typename CHAR = char, typename OFFSET = std::uint32_t> class Reader
{
public:
	using char_t = CHAR;
	using offset_t = OFFSET;


	/**
	 * Internal structure that hold references to tokens.s
	 */
	class TokenRef
	{
	private:
		offset_t mOffset; ///< Relative position in the data file.
		offset_t mLength; ///< Length of the token.
		offset_t mLineNumber; ///< Line number where the token is present.
		offset_t mCharNumber; ///< Index of the first token character on its respective line.

	public:
		TokenRef(offset_t offset, offset_t length, offset_t lineNumber, offset_t charNumber)
			: mOffset(offset), mLength(length), mLineNumber(lineNumber), mCharNumber(charNumber)
		{
		}

		offset_t offset() const
		{
			return mOffset;
		}

		offset_t length() const
		{
			return mLength;
		}

		offset_t lineNumber() const
		{
			return mLineNumber;
		}

		offset_t charNumber() const
		{
			return mCharNumber;
		}
	};


	/**
	 * Wrapper representing one parsed line of tokens.
	 * Provide various accessors to token data.
	 */
	class Line
	{
		friend class Reader<CHAR, OFFSET>;

	private:
		Reader<CHAR, OFFSET> &mReader;
		offset_t mLineNumber;
		std::vector<TokenRef> mTokens;
		const char_t *mRawData;
		offset_t mRawLength;

	public:
		Line(Reader<CHAR, OFFSET> &reader, offset_t lineNumber, const char_t *rawData, offset_t rawLength = 0)
			: mReader(reader), mLineNumber(lineNumber), mRawData(rawData), mRawLength(rawLength)
		{
		}


		/**
		 * Return const char pointer to the raw line data.
		 */
		const char_t *getRawLine() const
		{
			return mRawData;
		}


		/**
		 * Get the length of raw line string.
		 */
		offset_t getRawLength() const
		{
			return mRawLength;
		}


		/**
		 * Return the raw line as string object.
		 */
		std::string getRawLineAsString() const
		{
			return std::string(getRawLine(), getRawLength());
		}


		/**
		 * Get the number of the line in the original file.
		 */
		offset_t lineNumber() const
		{
			return mLineNumber;
		}

		/**
		 * Returns the number of tokens on the line.
		 */
		std::size_t size() const
		{
			return mTokens.size();
		}


		/**
		 * Raw accessor to underlying TokenRef objects.
		 */
		const TokenRef &operator[](std::size_t idx) const
		{
			return mTokens[idx];
		}


		/**
		 * Get raw token string as plain old const char pointer (not necesarly null-terminated).
		 */
		const char_t *getToken(std::size_t idx) const
		{
			return mReader.getToken(mTokens[idx]);
		}


		/**
		 * Return length of a token with given index.
		 */
		offset_t getTokenLength(std::size_t idx) const
		{
			return mTokens[idx].length();
		}


		/**
		 * Return the token as a copy wrapped in std string.
		 */
		std::string getTokenAsString(std::size_t idx) const
		{
			return std::string(getToken(idx), getTokenLength(idx));
		}
	};


private:
	bpp::MMapFile mFile; ///< Underlying mmaped file.
	bool mIgnoreEmptyLines; ///< Empty lines are skipped completely.
	bool mAllowComments; ///< Allow comments (lines starting with '#'), which are completely skipped.
	bool mIgnoreLineEnds; ///< Treat end lines as regular whitespace.
	bool mIgnoreTrailingWhitespace; ///< All whitespace (empty lines) at the end of the file is ignored

	char_t *mData; ///< Mmaped data of the file.
	offset_t mOffset; ///< Offset from the beginning of the file (currently processed).
	offset_t mLength; ///< Total length of the file.
	offset_t mLineNumber; ///< Number of current line.
	offset_t mLineOffset; ///< Offset of the beginning of current line.


	/**
	 * Whether the end of line has been reached.
	 */
	bool eol()
	{
		return !eof() && mData[mOffset] == (char) '\n';
	}


	/**
	 * Skip any whitespace characters except for newline.
	 */
	void skipWhitespace()
	{
		while (!eof() && !eol() && std::isspace((char) mData[mOffset])) ++mOffset;
	}


	/**
	 * Skip currently processed token (any non-whitespace characters).
	 */
	void skipToken()
	{
		while (!eof() && !std::isspace((char) mData[mOffset])) ++mOffset;
	}


	/**
	 * Skip all characters until the end of line (including the newline character).
	 */
	void skipRestOfLine()
	{
		while (!eof() && !eol()) ++mOffset;
		if (!eof()) ++mOffset; // skip newline char
		++mLineNumber;
		mLineOffset = mOffset;
	}


	/**
	 * Have we reached a comment start (if permitted)?
	 */
	bool isCommentStart()
	{
		return mAllowComments && !eof() && mData[mOffset] == (char_t) '#';
	}


	/**
	 * Have we reached a start of a token?
	 */
	bool isTokenStart()
	{
		return !eof() && !std::isspace((char) mData[mOffset]) && (!mAllowComments || mData[mOffset] != (char_t) '#');
	}


	/**
	 * Retrieve const char reference to a token.
	 */
	const char_t *getToken(const TokenRef &token)
	{
		return mData + token.offset();
	}

public:
	Reader(bool ignoreEmptyLines, bool allowComments, bool ignoreLineEnds, bool ignoreTrailingWhitespace)
		: mIgnoreEmptyLines(ignoreEmptyLines), mAllowComments(allowComments), mIgnoreLineEnds(ignoreLineEnds),
		  mIgnoreTrailingWhitespace(ignoreTrailingWhitespace), mData(nullptr), mOffset(0), mLength(0)
	{
	}


	/**
	 * Open memory mapped file and initilize reader.
	 */
	void open(const std::string &fileName)
	{
		mFile.open(fileName);
		if (mFile.length() > std::numeric_limits<offset_t>::max() &&
			mFile.length() > std::numeric_limits<offset_t>::max() * sizeof(char_t)) {
			throw(bpp::RuntimeError() << "File " << fileName
									  << " is too large to be loaded by current configuration of Reader.");
		}
		if (mFile.length() % sizeof(char_t) != 0) {
			throw(bpp::RuntimeError() << "File " << fileName << " size is not divisible by selected char size.");
		}

		mData = (char_t *) mFile.getData();
		mOffset = 0;
		mLength = mFile.length() / sizeof(char_t);
		mLineNumber = 1;
		mLineOffset = 0;

		if (mIgnoreTrailingWhitespace) {
			// Reduce the file length to ignore all whitespace at the end ...
			while (mLength > 0 && std::isspace(mData[mLength - 1])) {
				--mLength;
			}
		}
	}


	/**
	 * Is the reader open for reading?
	 */
	bool opened() const
	{
		return mFile.opened();
	}


	/**
	 * Close the reader and underlying mmapped file.
	 */
	void close()
	{
		mFile.close();
		mData = nullptr;
		mOffset = mLength = 0;
	}


	/**
	 * Whether the end of file has been reached.
	 */
	bool eof()
	{
		return mOffset >= mLength;
	}


	/**
	 * Parse one line of tokens. If new lines are ignored, entire file is parsed.
	 * \return Unique pointer to a Line object.
	 */
	std::unique_ptr<Line> readLine()
	{
		if (eof()) {
			return std::unique_ptr<Line>();
		}

		offset_t startOffset = mOffset;
		auto line = bpp::make_unique<Line>(*this, mLineNumber, mData + mOffset);
		while (!eof()) {
			skipWhitespace();

			if (isTokenStart()) {
				// A regular token was encountered -- add it to the list.
				offset_t start = mOffset;
				skipToken();
				line->mTokens.push_back(TokenRef(start, mOffset - start, mLineNumber, start - mLineOffset + 1));
				continue; // let's go read another token
			} else if (!isCommentStart() && !eol() && !eof()) {
				throw bpp::RuntimeError("Something is wrong since this Reader state is deamed impossible.");
			}

			// Here we are at the end of a line or start of a comment ...
			bool comment = isCommentStart();
			skipRestOfLine();
			if (mIgnoreLineEnds) continue; // new lines are ignored, lets continue read tokens
			if (!line->mTokens.empty() || (!mIgnoreEmptyLines && !comment))
				break; // line is non-empty or we return empty lines

			// If we got here, an empty line or a comment line was read (which we skipped).
			line->mLineNumber = mLineNumber;
			line->mRawData = mData + mOffset;
			startOffset = mOffset;
		}

		if (line->mTokens.empty() && mIgnoreEmptyLines) {
			// The last line of the file was empty, we should skip it as well ...
			return std::unique_ptr<Line>();
		}

		line->mRawLength =
			line->mTokens.size() > 0 ? line->mTokens.back().charNumber() + line->mTokens.back().length() : 0;
		return line;
	}
};


#endif

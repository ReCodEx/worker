#ifndef RECODEX_TOKEN_JUDGE_JUDGE_HPP
#define RECODEX_TOKEN_JUDGE_JUDGE_HPP


#include <misc/exception.hpp>

#include <vector>
#include <string>
#include <memory>
#include <cstddef>


/**
 * Encapsulate an algorithm for comparing lines like a diff utility does.
 */
template <class READER, class LINE_COMPARATOR> class Judge
{
public:
	typedef READER reader_t;
	typedef LINE_COMPARATOR line_comparator_t;

private:
	bool mShuffledLines;
	reader_t &mCorrectReader;
	reader_t &mResultReader;
	line_comparator_t mLineComparator;

	std::unique_ptr<typename reader_t::Line> mCorrectLine;
	std::unique_ptr<typename reader_t::Line> mResultLine;

	std::vector<std::unique_ptr<typename reader_t::Line>> mCorrectLinesBuffer;
	std::vector<std::unique_ptr<typename reader_t::Line>> mResultLinesBuffer;


	void readNextCorrectLine()
	{
		mCorrectLine = mCorrectReader.readLine();
		if (!mCorrectLine) {
			throw bpp::RuntimeError("Error has occured while reading the file with correct answers.");
		}
	}

	void readNextResultLine()
	{
		mResultLine = mResultReader.readLine();
		if (!mResultLine) {
			throw bpp::RuntimeError("Error has occured while reading the results file which is being tested.");
		}
	}

	void readNextLines()
	{
		readNextCorrectLine();
		readNextResultLine();
	}


	bool skipMatchingLeadingLines()
	{
		while (!mCorrectReader.eof() && !mResultReader.eof()) {
			// Read following lines from the readers.
			readNextLines();
			if (!mLineComparator.compare(*mCorrectLine.get(), *mResultLine.get())) return false;
		}
		return true;
	}

	bool compareOrdered()
	{
		bool lastLinesMatching = skipMatchingLeadingLines();
		if (lastLinesMatching && mCorrectReader.eof() && mResultReader.eof()) return true;

		return false;
	}

	bool compareUnordered()
	{
		throw bpp::RuntimeError("Not implemented yet.");
	}

public:
	Judge(bool shuffledLines, READER &correctReader, READER &resultReader, LINE_COMPARATOR &lineComparator)
		: mShuffledLines(shuffledLines), mCorrectReader(correctReader), mResultReader(resultReader),
		  mLineComparator(lineComparator)
	{
	}


	bool compare()
	{
		return mShuffledLines ? compareUnordered() : compareOrdered();
	}
};


#endif
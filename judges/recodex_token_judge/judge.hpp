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
	typedef typename READER::Line line_t;
	typedef LINE_COMPARATOR line_comparator_t;
	typedef typename LINE_COMPARATOR::result_t score_t;

private:
	/**
	 * Record kept in each matrix cell for LCS dynamic-programming algorithm ...
	 */
	struct LCSNode {
		score_t comparisonResult; ///< Result from the comparator of the corresonding two lines.
		score_t score; ///< Actual total score of the LCS found in this node.

		// relative offset to previous node
		std::int16_t dr;
		std::int16_t dc;
	};

	/**
	 * Record for diff results reconstructed by comparing LCS with actual rows.
	 */
	struct Diff {
		static const std::size_t NO_IDX = ~(std::size_t) 0;
		std::size_t correct, result; //< Indices to line buffers (~0 means no index)
		Diff(std::size_t _correct, std::size_t _result) : correct(_correct), result(_result)
		{
		}
	};


	bool mShuffledLines;
	reader_t &mCorrectReader;
	reader_t &mResultReader;
	line_comparator_t mLineComparator;

	std::unique_ptr<typename reader_t::Line> mCorrectLine;
	std::unique_ptr<typename reader_t::Line> mResultLine;

	std::vector<std::unique_ptr<typename reader_t::Line>> mCorrectLinesBuffer;
	std::vector<std::unique_ptr<typename reader_t::Line>> mResultLinesBuffer;


	/**
	 *
	 */
	void readNextCorrectLine()
	{
		if (mCorrectLinesBuffer.empty()) {
			mCorrectLine = mCorrectReader.readLine();
		} else {
			// If the buffer is not empty, return its first line and remove it from the buffer...
			mCorrectLine = std::move(mCorrectLinesBuffer.front());
			mCorrectLinesBuffer.erase(mCorrectLinesBuffer.begin());
		}
	}


	/**
	 *
	 */
	void readNextResultLine()
	{
		if (mResultLinesBuffer.empty()) {
			mResultLine = mResultReader.readLine();
		} else {
			// If the buffer is not empty, return its first line and remove it from the buffer...
			mResultLine = std::move(mResultLinesBuffer.front());
			mResultLinesBuffer.erase(mResultLinesBuffer.begin());
		}
	}


	/**
	 *
	 */
	void readNextLines()
	{
		readNextCorrectLine();
		readNextResultLine();
	}


	/**
	 *
	 */
	bool skipMatchingLeadingLines()
	{
		while ((!mCorrectReader.eof() || !mCorrectLinesBuffer.empty()) &&
			(!mResultReader.eof() || !mResultLinesBuffer.empty())) {
			// Read following lines from the readers.
			readNextLines();
			if (!mCorrectLine && !mResultLine) return true; // both files have termineated
			if (!mCorrectLine || !mResultLine) return false; // one of the files have termineated
			if (mLineComparator.compare(*mCorrectLine.get(), *mResultLine.get()) != 0) return false;
		}
		return true;
	}


	/**
	 * Read reasonable amount of lines to both correct and result line buffers.
	 * The amount of data read is based on internal limits for lines, tokens, and chars being read.
	 */
	void fillBuffers()
	{
		const std::size_t MAX_LINES = 100; // amount of lines affect complexity of lcs algorithm
		const std::size_t MAX_TOKENS = 1000; // amount of tokens affect aggregated complexities of line comparisons
		const std::size_t MAX_CHARS = 10000; // amount of chars affect aggregated complexities of token comarisons

		// Fill in the first correct line which already has been loaded.
		if (mCorrectLine) {
			mCorrectLinesBuffer.insert(mCorrectLinesBuffer.begin(), std::move(mCorrectLine));
		}

		// Count stats of actual state of the correct lines buffer...
		std::size_t tokens = 0, chars = 0;
		for (auto &&it : mCorrectLinesBuffer) {
			tokens += it->size();
			chars += it->getRawLength();
		}

		// Read correct file until the end or the limits are reached.
		while (!mCorrectReader.eof() && mCorrectLinesBuffer.size() < MAX_LINES && tokens < MAX_TOKENS &&
			chars < MAX_CHARS) {
			mCorrectLinesBuffer.push_back(mCorrectReader.readLine());
			tokens += mCorrectLinesBuffer.back()->size();
			chars += mCorrectLinesBuffer.back()->getRawLength();
		}

		// Fill in the first result line which already has been loaded.
		if (mResultLine) {
			mResultLinesBuffer.insert(mResultLinesBuffer.begin(), std::move(mResultLine));
		}

		// Count stats of actual state of the result lines buffer...
		tokens = chars = 0;
		for (auto &&it : mResultLinesBuffer) {
			tokens += it->size();
			chars += it->getRawLength();
		}

		// Read result file until the end or the limits are reached.
		while (
			!mResultReader.eof() && mResultLinesBuffer.size() < MAX_LINES && tokens < MAX_TOKENS && chars < MAX_CHARS) {
			mResultLinesBuffer.push_back(mResultReader.readLine());
			tokens += mResultLinesBuffer.back()->size();
			chars += mResultLinesBuffer.back()->getRawLength();
		}
	}


	/**
	 *
	 */
	void computeLCSMatrix(std::vector<LCSNode> &lcsMatrix)
	{
		std::size_t sizeC = mCorrectLinesBuffer.size();
		std::size_t sizeR = mResultLinesBuffer.size();

		// Prepare initial LCS matrix and top-left boundaries ...
		lcsMatrix.clear();
		lcsMatrix.resize((sizeC + 1) * (sizeR + 1));
		for (std::size_t c = 0; c < sizeC; ++c) {
			lcsMatrix[(c + 1) * (sizeR + 1)].score =
				lcsMatrix[c * (sizeR + 1)].score + mCorrectLinesBuffer[c]->size() + 1;
			lcsMatrix[(c + 1) * (sizeR + 1)].dc = -1;
		}
		for (std::size_t r = 0; r < sizeR; ++r) {
			lcsMatrix[r + 1].score = lcsMatrix[r].score + mResultLinesBuffer[r]->size() + 1;
			lcsMatrix[r + 1].dr = -1;
		}


		// Fill in the LCS matrix by dynamic programming
		std::size_t i = sizeR + 2; // current position in matrix (i == (c+1)*(sizeR+1) + (r+1))
		for (std::size_t c = 0; c < sizeC; ++c) {
			for (std::size_t r = 0; r < sizeR; ++r) {
				lcsMatrix[i].comparisonResult =
					mLineComparator.compare(*mCorrectLinesBuffer[c].get(), *mResultLinesBuffer[r].get());

				// Compute score for each of three possibilities ...
				score_t upperScore = lcsMatrix[i - sizeR - 1].score + mCorrectLinesBuffer[c]->size() + 1;
				score_t leftScore = lcsMatrix[i - 1].score + mResultLinesBuffer[r]->size() + 1;
				score_t upperLeftScore = lcsMatrix[i - sizeR - 2].score + lcsMatrix[i].comparisonResult;

				// Find the best option (with the lowest score).
				if (upperLeftScore <= leftScore && upperLeftScore <= upperScore) {
					lcsMatrix[i].dr = -1;
					lcsMatrix[i].dc = -1;
					lcsMatrix[i].score = upperLeftScore;
				} else if (leftScore <= upperScore) {
					lcsMatrix[i].dr = -1;
					lcsMatrix[i].score = leftScore;

				} else {
					lcsMatrix[i].dc = -1;
					lcsMatrix[i].score = upperScore;
				}

				++i;
			}
			++i; // skip the first (padding) column
		}
	}


	/**
	 * 
	 */
	void collectDiffRecords(const std::vector<LCSNode> &lcsMatrix,
		std::vector<Diff> &diff,
		std::size_t &lastMatchedCorrect,
		std::size_t &lastMatchedResult)
	{
		std::size_t sizeC = mCorrectLinesBuffer.size();
		std::size_t sizeR = mResultLinesBuffer.size();
		std::size_t c = sizeC;
		std::size_t r = sizeR;

		while (c > 0 || r > 0) {
			const LCSNode &node = lcsMatrix[c * (sizeR + 1) + r];

			if (node.dc == 0 || node.dr == 0 ||
				node.comparisonResult != 0) { // either one line is skipped or the the lines do not match completely
				diff.push_back(Diff(node.dc ? c - 1 : Diff::NO_IDX, node.dr ? r - 1 : Diff::NO_IDX));
			} else if (node.dc != 0 && node.dr != 0 && node.comparisonResult == 0) {
				if (lastMatchedCorrect == Diff::NO_IDX) lastMatchedCorrect = c - 1;
				if (lastMatchedResult == Diff::NO_IDX) lastMatchedResult = r - 1;
			}

			c += node.dc;
			r += node.dr;
		}
	}


	/**
	 *
	 */
	void logImpairedCorrectLine(const line_t &line) const
	{
		bpp::log().error() << "-" << line.lineNumber() << ": " << line.getRawLineAsString();
	}


	/**
	 *
	 */
	void logImpairedResultLine(const line_t &line) const
	{
		bpp::log().error() << "+" << line.lineNumber() << ": " << line.getRawLineAsString();
	}


	/**
	 *
	 */
	void processAndLogDiffs(
		const std::vector<Diff> &diff, std::size_t lastMatchedCorrect, std::size_t lastMatchedResult)

	{
		std::size_t i = diff.size();
		std::size_t lastCorrect = Diff::NO_IDX;
		std::size_t lastResult = Diff::NO_IDX;
		while (i > 0) {
			--i;
			if (diff[i].correct != Diff::NO_IDX && diff[i].result != Diff::NO_IDX) {
				// Lines are matched but not entirely the same, re-check them and print out the differences...
				mLineComparator.compareAndLog(
					*mCorrectLinesBuffer[diff[i].correct].get(), *mResultLinesBuffer[diff[i].result].get());
				lastCorrect = diff[i].correct;
				lastResult = diff[i].result;
			} else if (diff[i].correct != Diff::NO_IDX) {
				// Correct line was skipped...
				logImpairedCorrectLine(*mCorrectLinesBuffer[diff[i].correct].get());
				lastCorrect = diff[i].correct;
			} else {
				// Result line was skipped...
				logImpairedResultLine(*mResultLinesBuffer[diff[i].result].get());
				lastResult = diff[i].result;
			}
			if (diff[i].correct == mCorrectLinesBuffer.size() - 1 || diff[i].result == mResultLinesBuffer.size() - 1)
				break;
		}

		if (lastMatchedCorrect != Diff::NO_IDX) {
			lastCorrect =
				(lastCorrect != Diff::NO_IDX) ? std::max(lastCorrect, lastMatchedCorrect) : lastMatchedCorrect;
		}

		if (lastMatchedResult != Diff::NO_IDX) {
			lastResult = (lastResult != Diff::NO_IDX) ? std::max(lastResult, lastMatchedResult) : lastMatchedResult;
		}

		if (lastCorrect != Diff::NO_IDX) {
			mCorrectLinesBuffer.erase(mCorrectLinesBuffer.begin(), mCorrectLinesBuffer.begin() + lastCorrect + 1);
		}
		if (lastResult != Diff::NO_IDX) {
			mResultLinesBuffer.erase(mResultLinesBuffer.begin(), mResultLinesBuffer.begin() + lastResult + 1);
		}
	}


	/**
	 *
	 */
	bool logImpairedCorrectTrailing()
	{
		// Check correct file remains ...
		bool reportedAny = false;
		while (!mCorrectReader.eof() && !bpp::log().isFull(bpp::LogSeverity::ERROR)) {
			readNextCorrectLine();
			if (mCorrectLine) {
				logImpairedCorrectLine(*mCorrectLine.get());
				reportedAny = true;
			}
		}
		return reportedAny;
	}


	/**
	 *
	 */
	bool logImpairedResultTrailing()
	{
		// Check result file remains ...
		bool reportedAny = false;
		while (!mResultReader.eof() && !bpp::log().isFull(bpp::LogSeverity::ERROR)) {
			readNextResultLine();
			if (mResultLine) {
				logImpairedResultLine(*mResultLine.get());
				reportedAny = true;
			}
		}
		return reportedAny;
	}


	/**
	 *
	 */
	bool compareOrdered()
	{
		// Try to skip leading portions of files which match completely (skips entire files if result is
		// correct).
		bool lastLinesMatching = skipMatchingLeadingLines();
		if (lastLinesMatching && mCorrectReader.eof() && mResultReader.eof()) return true;

		if (!lastLinesMatching) {
			while (!bpp::log().isFull(bpp::LogSeverity::ERROR)) {
				std::vector<LCSNode> lcsMatrix;
				std::vector<Diff> diff;
				fillBuffers();

				// If one of the buffers is empty, wrap it up ...
				if ((mCorrectReader.eof() && mCorrectLinesBuffer.empty()) || (mResultReader.eof() && mResultLinesBuffer.empty())) {
					logImpairedCorrectTrailing();
					logImpairedResultTrailing();
					break;
				}

				computeLCSMatrix(lcsMatrix);

				std::size_t lastMatchedCorrect = Diff::NO_IDX;
				std::size_t lastMatchedResult = Diff::NO_IDX;
				collectDiffRecords(lcsMatrix, diff, lastMatchedCorrect, lastMatchedResult);
				processAndLogDiffs(diff, lastMatchedCorrect, lastMatchedResult);

				if (mCorrectReader.eof() && mCorrectLinesBuffer.empty() && mResultReader.eof() &&
					mResultLinesBuffer.empty())
					break;
			}

			return false;
		} else {
			return !logImpairedCorrectTrailing() && !logImpairedResultTrailing();			
		}
	}


	/**
	 *
	 */
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


	/**
	 * Run the judge algorithm based on given readers and line comparator using either ordered or unordered
	 * strategy. Any mismatches are duly loged in global bpp::log(). \return True if both files match according
	 * to given parameters, false otherwise.
	 */
	bool compare()
	{
		return mShuffledLines ? compareUnordered() : compareOrdered();
	}
};


#endif
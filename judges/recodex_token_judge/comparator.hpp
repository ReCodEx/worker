#ifndef RECODEX_TOKEN_JUDGE_COMPARATOR_HPP
#define RECODEX_TOKEN_JUDGE_COMPARATOR_HPP

#include "reader.hpp"

#include <algo/lcs.hpp>
#include <cli/logger.hpp>

#include <map>
#include <algorithm>
#include <string>
#include <limits>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <cstdint>


/**
 * Try to parse long int out of a string. Return true if the string contains only an integer, false otherwise.
 * \param str String to be parsed.
 * \param res Reference to a variable where the parsed number will be stored.
 */
template <typename STRING> bool try_get_int(const STRING &str, long int &res)
{
	char *end;
	res = std::strtol(str.c_str(), &end, 10);
	return (end - str.c_str() == (int) str.length());
}


/**
 * Try to parse double out of a string. Return true if the string contains only a double, false otherwise.
 * \param str String to be parsed.
 * \param res Reference to a variable where the parsed number will be stored.
 */
template <typename STRING> bool try_get_double(const STRING &str, double &res)
{
	char *end;
	res = std::strtod(str.c_str(), &end);
	return (end - str.c_str() == (int) str.length());
}


/**
 * Comparator that compares tokens for equality based on given configuration switches.
 */
template <typename CHAR = char, typename OFFSET = std::uint32_t> class TokenComparator
{
public:
	typedef CHAR char_t;
	typedef OFFSET offset_t;

private:
	/**
	 * Internal structure where tokens are loaded if more commplex comparison is required.
	 */
	struct TokenPair {
		typedef std::basic_string<CHAR, std::char_traits<CHAR>, std::allocator<CHAR>> string_t;

	public:
		string_t token[2];

		TokenPair(const char_t *t1, offset_t len1, const char_t *t2, offset_t len2)
		{
			token[0].assign(t1, len1);
			token[1].assign(t2, len2);
		}

		string_t &first()
		{
			return token[0];
		}

		const string_t &first() const
		{
			return token[0];
		}

		string_t &second()
		{
			return token[1];
		}

		const string_t &second() const
		{
			return token[1];
		}


		/**
		 * Atempt to parse both tokens as integers.
		 * \param num1 Result number of the first token.
		 * \param num2 Result number of the second token.
		 * \return True if both values are ints, false otherwise.
		 */
		bool tryGetInts(long int &num1, long int &num2)
		{
			return try_get_int(first(), num1) && try_get_int(second(), num2);
		}


		/**
		 * Atempt to parse both tokens as doubles.
		 * \param num1 Result number of the first token.
		 * \param num2 Result number of the second token.
		 * \return True if both values are doubles, false otherwise.
		 */
		bool tryGetFloats(double &num1, double &num2)
		{
			return try_get_double(first(), num1) && try_get_double(second(), num2);
		}
	};


	/**
	 * Direct comparison of both strings as const-chars. Saves time as the const chars may point directly to mmaped
	 * data.
	 */
	bool compareDirect(const char_t *t1, offset_t len1, const char_t *t2, offset_t len2) const
	{
		if (len1 != len2) return false;
		for (offset_t i = 0; i < len1; ++i) {
			if (t1[i] != t2[i]) return false;
		}
		return true;
	}


	/**
	 * Direct comparison of both strings as const-chars. Saves time as the const chars may point directly to mmaped
	 * data. Each char is lowercased just before comparison.
	 */
	bool compareDirectLowercased(const char_t *t1, offset_t len1, const char_t *t2, offset_t len2) const
	{
		if (len1 != len2) return false;
		for (offset_t i = 0; i < len1; ++i) {
			if (std::tolower(t1[i]) != std::tolower(t2[i])) return false;
		}
		return true;
	}


	bool mIgnoreCase; ///< Flag indicating that token comparisons should ignore letter case.
	bool mNumeric; ///< Flag indicating that the comparator should compare numeric tokens (ints and doubles) as numbers.
	double mFloatTolerance; ///< Float tolerance used for double tokens when numeric comparisons are allowed.

public:
	TokenComparator(bool ignoreCase = false, bool numeric = false, double floatTolerance = 0.0)
		: mIgnoreCase(ignoreCase), mNumeric(numeric), mFloatTolerance(floatTolerance)
	{
	}

	bool ignoreCase() const
	{
		return mIgnoreCase;
	}

	bool numeric() const
	{
		return mNumeric;
	}

	double floatTolerance() const
	{
		return mFloatTolerance;
	}


	/**
	 * The main function that compares two tokens based on internal flags.
	 * \param t1 Pointer to the raw data representing the first token.
	 * \param len1 Number of characters of the first token (tokens are not null-terminated).
	 * \param t2 Pointer to the raw data representing the second token.
	 * \param len2 Number of characters of the second token (tokens are not null-terminated).
	 * \return True if the tokens are matching, false otherwise.
	 */
	bool compare(const char_t *t1, offset_t len1, const char_t *t2, offset_t len2) const
	{
		if (mNumeric && len1 < 32 && len2 < 32) { // no number should have more than 32 chars
			TokenPair tokenPair(t1, len1, t2, len2);

			long int i1, i2;
			if (tokenPair.tryGetInts(i1, i2)) {
				return i1 == i2;
			}

			double d1, d2;
			if (tokenPair.tryGetFloats(d1, d2)) {
				double err = std::abs(d1 - d2) / std::abs(d1 + d2);
				return err <= mFloatTolerance;
			}
		}

		return mIgnoreCase ? compareDirectLowercased(t1, len1, t2, len2) : compareDirect(t1, len1, t2, len2);
	}
};


/**
 * Comparator that compares two lines of tokens.
 */
template <typename CHAR = char, typename OFFSET = std::uint32_t, typename RESULT = std::uint32_t> class LineComparator
{
public:
	typedef CHAR char_t;
	typedef OFFSET offset_t;
	typedef RESULT result_t;
	typedef typename Reader<CHAR, OFFSET>::Line line_t;
	typedef typename Reader<CHAR, OFFSET>::TokenRef token_t;

private:
	TokenComparator<CHAR, OFFSET> &mTokenComparator; ///< Token comparator used for comparing tokens on the lines.
	bool mShuffledTokens; ///< Whether the tokens on each line may be in arbitrary order.

	/**
	 * Log one error of unordered token comparison (a token is superfluous or missing).
	 * \tparam T Type of the token value.
	 * \param value The value to be logged.
	 * \param diff The difference (how many times the token was recorded or expected).
	 *        If lower than zero, token was missing, otherwise the token is supefluous.
	 * \param line The index of the line where the error occured.
	 * \param quote Whether the token value should be quoted (strings are quoted, ints and floats are not).
	 */
	template <typename T> void logUnorderedError(const T &value, int diff, offset_t line, bool quote) const
	{
		static offset_t lastLineErrorLogged = 0;

		if (lastLineErrorLogged != line) {
			bpp::log().error() << line << ": ";
			lastLineErrorLogged = line;
		} else {
			bpp::log().error() << "\t";
		}

		if (diff < 0) {
			bpp::log().error() << "unexpected " << (quote ? "'" : "") << value << (quote ? "'" : "");
		} else {
			bpp::log().error() << "missing " << (quote ? "'" : "") << value << (quote ? "'" : "");
		}

		if (std::abs(diff) > 1) {
			bpp::log().warning() << " (" << std::abs(diff) << "x)";
		}
		bpp::log().error() << "\n";
	}


	/**
	 * Perform verification of map of valued from the unordered comparison and log all errors.
	 * \tparam T Type of the token value.
	 * \tparam LOGGING If false, the check is performed silently. Otherwise, the errors are logged using bpp::log().
	 * \param mapValues The the map of values and their diffs to be checked.
	 * \param line The index of the line where the error occured.
	 * \param quote Whether the token value should be quoted (strings are quoted, ints and floats are not).
	 */
	template <typename T, bool LOGGING>
	result_t checkMapValues(const std::map<T, int> &mapValues, offset_t line, bool quote) const
	{
		result_t errorCount = 0;
		for (auto &&it : mapValues) {
			errorCount += std::abs(it.second);
			if (LOGGING) {
				logUnorderedError(it.first, it.second, line, quote);
			}
		}
		return errorCount;
	}


	/**
	 * Perform the unordered comparison (the tokens on line may be in any order).
	 * \tparam LOGGING If false, the check is performed silently. Otherwise, the errors are logged using bpp::log().
	 * \param line1 The first line (correct) passed from the reader.
	 * \param line2 The second line (result) passed from the reader.
	 * \return Number of errors (i.e., 0 == lines match completely).
	 */
	template <bool LOGGING = false> result_t compareUnordered(const line_t &line1, const line_t &line2) const
	{
		std::map<std::string, int> stringTokens;
		std::map<long int, int> intTokens;
		std::map<double, int> doubleTokens;

		// Fill in the sets with the first line ...
		for (offset_t i = 0; line1.size(); ++i) {
			std::string token = line1.getTokenAsString(i);
			if (mTokenComparator.numeric()) {
				long int ival;
				double dval;
				if (try_get_int(token, ival)) {
					intTokens[ival]++;
				} else if (try_get_double(token, dval)) {
					doubleTokens[dval]++;
				} else {
					stringTokens[token]++;
				}
			} else {
				stringTokens[token]++;
			}
		}

		// Compare the second line with assets stored in maps ...
		for (offset_t i = 0; line2.size(); ++i) {
			std::string token = line2.getTokenAsString(i);
			if (mTokenComparator.numeric()) {
				long int ival;
				double dval;
				if (try_get_int(token, ival)) {
					intTokens[ival]--;
				} else if (try_get_double(token, dval)) {
					doubleTokens[dval]--;
				} else {
					stringTokens[token]--;
				}
			} else {
				stringTokens[token]--;
			}
		}

		result_t errorCount = checkMapValues<std::string, LOGGING>(stringTokens, line2.lineNumber(), true);
		if (mTokenComparator.numeric()) {
			errorCount += checkMapValues<long int, LOGGING>(intTokens, line2.lineNumber(), false);
			errorCount += checkMapValues<double, LOGGING>(doubleTokens, line2.lineNumber(), false);
		}

		return (result_t) errorCount;
	}


	/**
	 * Log one missing or unexpected token on an ordered line.
	 * \param token The token to be logged (metadata only).
	 * \param value The string value of the token.
	 * \param sign The sign to be printed in the log (+ or -).
	 */
	void logOrderedError(const token_t &token, const std::string &value, const char *sign) const
	{
		bpp::log().error() << " " << sign << "[" << token.charNumber() << "]" << value;
	}


	/**
	 * Log a mismatching pair of tokens.
	 * \param token1 The first (correct) token to be logged (metadata only).
	 * \param value1 The string value of the first token.
	 * \param token2 The second (result) token to be logged (metadata only).
	 * \param value2 The string value of the second token.
	 */
	void logMismatchError(
		const token_t &token1, const std::string &value1, const token_t &token2, const std::string &value2) const
	{
		bpp::log().error() << " [" << token1.charNumber() << "]" << value1 << " != [" << token2.charNumber() << "]"
						   << value2;
	}


	/**
	 * Log the errors from ordered line comparison. The errors are logged as missing or missmatched tokens
	 * between current position c,r and next matching position toC,toR.
	 * \param line1 The first line being compared (correct).
	 * \param line2 The second line being compared (result).
	 * \param c Index of the current token being logged on the first line.
	 * \param r Index of the current token being logged on the second line.
	 * \param toC Index of the next matched token on the first line.
	 * \param toR Index of the next matched token on the second line.
	 */
	void logOrderedErrors(const line_t &line1,
		const line_t &line2,
		std::size_t &c,
		std::size_t &r,
		std::size_t toC,
		std::size_t toR) const
	{
		while (c < toC && r < toR) {
			// Log the errors as token mismatchs
			logMismatchError(line1[c], line1.getTokenAsString(c), line2[r], line2.getTokenAsString(r));
			++c;
			++r;
		}

		while (c < toC) {
			// Missing tokens (present in correct file, but missing in results).
			logOrderedError(line1[c], line1.getTokenAsString(c), "-");
			++c;
		}

		while (r < toR) {
			// Missing tokens (present in correct file, but missing in results).
			logOrderedError(line2[r], line2.getTokenAsString(r), "+");
			++r;
		}
	}


	/**
	 * Apply LCS algorithm to find the best matching between the two lines
	 * and determine the error as the number of tokens not present in the common subequence.
	 * \tparam LOGGING If false, the check is performed silently. Otherwise, the errors are logged using bpp::log().
	 * \param line1 The first line (correct) passed from the reader.
	 * \param line2 The second line (result) passed from the reader.
	 * \return Number of errors (i.e., 0 == lines match completely).
	 */
	template <bool LOGGING = false> result_t compareOrdered(const line_t &line1, const line_t &line2) const
	{
		TokenComparator<CHAR, OFFSET> &comparator = mTokenComparator;
		if (LOGGING) {
			std::vector<std::pair<std::size_t, std::size_t>> lcs;
			bpp::longest_common_subsequence(line1,
				line2,
				lcs,
				[&comparator](const line_t &line1, std::size_t i1, const line_t &line2, std::size_t i2) {
					return comparator.compare(
						line1.getToken(i1), line1[i1].length(), line2.getToken(i2), line2[i2].length());
				});

			// If there are no errors, return immediately.
			result_t res = (result_t)(line1.size() - lcs.size() + line2.size() - lcs.size());
			if (res == 0) return res;

			bpp::log().error() << "-" << line1.lineNumber() << "/+" << line2.lineNumber() << ":";

			std::size_t c = 0;
			std::size_t r = 0;
			for (auto &&it : lcs) {
				logOrderedErrors(line1, line2, c, r, it.first, it.second);

				// Skip the matched pair ...
				++c;
				++r;
			}

			// Log trailing tokens after last matched pair.
			logOrderedErrors(line1, line2, c, r, line1.size(), line2.size());

			bpp::log().error() << "\n";
			return res;
		} else {
			std::size_t lcs = bpp::longest_common_subsequence_length(
				line1, line2, [&comparator](const line_t &line1, std::size_t i1, const line_t &line2, std::size_t i2) {
					return comparator.compare(
						line1.getToken(i1), line1[i1].length(), line2.getToken(i2), line2[i2].length());
				});
			return (result_t)(line1.size() - lcs + line2.size() - lcs);
		}
	}


public:
	LineComparator(TokenComparator<CHAR, OFFSET> &tokenComparator, bool shuffledTokens)
		: mTokenComparator(tokenComparator), mShuffledTokens(shuffledTokens)
	{
	}

	/**
	 * Compare the lines only and return the number of errors.
	 * \return Zero if the lines match completely, number of mismatched tokens otherwise.
	 */
	result_t compare(const line_t &line1, const line_t &line2) const
	{
		return (mShuffledTokens) ? compareUnordered<false>(line1, line2) : compareOrdered<false>(line1, line2);
	}


	/**
	 * Compare the lines and log all the mismatched tokens to global log.
	 * \return Zero if the lines match completely, number of mismatched tokens otherwise.
	 */
	result_t compareAndLog(const line_t &line1, const line_t &line2) const
	{
		return (mShuffledTokens) ? compareUnordered<true>(line1, line2) : compareOrdered<true>(line1, line2);
	}
};

#endif

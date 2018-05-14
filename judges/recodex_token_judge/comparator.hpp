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
	return (end - str.c_str() == str.length());
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
	return (end - str.c_str() == str.length());
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


	bool mIgnoreCase;        ///< Flag indicating that token comparisons should ignore letter case.
	bool mNumeric;           ///< Flag indicating that the comparator should compare numeric tokens (ints and doubles) as numbers.
	double mFloatTolerance;  ///< Float tolerance used for double tokens when numeric comparisons are allowed.

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

private:
	TokenComparator<CHAR, OFFSET> &mTokenComparator;  ///< Token comparator used for comparing tokens on the lines.
	bool mShuffledTokens;                             ///< Whether the tokens on each line may be in arbitrary order. 

	static result_t computeResult(std::size_t errors, std::size_t total)
	{
		double res = (double) std::numeric_limits<result_t>::max() * (double) errors / (double) total;
		return (result_t) std::round(res);
	}

	template <typename T>
	void logError(const T &value, int diff, offset_t line, const std::string &caption = "token") const
	{
		static offset_t lastLineErrorLogged = 0;

		if (lastLineErrorLogged != line) {
			bpp::log().error() << line << ": ";
			lastLineErrorLogged = line;
		} else {
			bpp::log().error() << "\t";
		}

		if (diff < 0) {
			bpp::log().error() << "unexpected " << caption << " '" << value << "'";
		} else {
			bpp::log().error() << "missing " << caption << " '" << value << "'";
		}

		if (std::abs(diff) > 1) {
			bpp::log().warning() << " (" << std::abs(diff) << "x)";
		}
		bpp::log().error() << "\n";
	}


	template <typename T, bool LOGGING>
	std::size_t checkMapValues(
		const std::map<T, int> &mapValues, offset_t line, const std::string &caption = "token") const
	{
		std::size_t errorCount = 0;
		for (auto &&it : mapValues) {
			if (it.second != 0) ++errorCount;
			if (LOGGING) {
				logError(it.first, it.second, line, caption);
			}
		}
		return errorCount;
	}


	template <bool LOGGING = false> result_t compareUnordered(const line_t &line1, const line_t &line2) const
	{
		std::map<std::string, int> stringTokens;
		std::map<long int, int> intTokens;
		std::map<double, int> doubleTokens;

		// Fill in the sets with the first line ...
		for (offset_t i = 0; line1.size(); ++i) {
			std::string token = line1.getString(i);
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
			std::string token = line2.getString(i);
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

		std::size_t errorCount = checkMapValues<std::string, LOGGING>(stringTokens, line2.lineNumber(), "token");
		if (mTokenComparator.numeric()) {
			errorCount += checkMapValues<long int, LOGGING>(intTokens, line2.lineNumber(), "int");
			errorCount += checkMapValues<double, LOGGING>(doubleTokens, line2.lineNumber(), "float");
		}

		return computeResult(errorCount, line1.size() + line2.size());
	}


	template <bool LOGGING = false> result_t compareOrdered(const line_t &line1, const line_t &line2) const
	{
		TokenComparator<CHAR, OFFSET> &comparator = mTokenComparator;
		std::size_t lcs = bpp::longest_common_subsequence_length(
			line1, line2, [&comparator](const line_t &line1, std::size_t i1, const line_t &line2, std::size_t i2) {
				return comparator.compare(
					line1.getCString(i1), line1[i1].length(), line2.getCString(i2), line2[i2].length());
			});
		return computeResult(lcs - line1.size() + lcs - line2.size(), line1.size() + line2.size());
	}


public:
	LineComparator(TokenComparator<CHAR, OFFSET> &tokenComparator, bool shuffledTokens)
		: mTokenComparator(tokenComparator), mShuffledTokens(shuffledTokens)
	{
	}

	result_t compare(const line_t &line1, const line_t &line2) const
	{
		return (mShuffledTokens) ? compareUnordered<false>(line1, line2) : compareOrdered<false>(line1, line2);
	}

	result_t compareAndLog(const line_t &line1, const line_t &line2) const
	{
		return (mShuffledTokens) ? compareUnordered<true>(line1, line2) : compareOrdered<true>(line1, line2);
	}
};

#endif

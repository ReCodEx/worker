/*
* Author: Martin Krulis <krulis@ksi.mff.cuni.cz>
* Last Modification: 14.5.2018
* License: CC 3.0 BY-NC (http://creativecommons.org/)
*/
#ifndef BPPLIB_ALGO_LCS_HPP
#define BPPLIB_ALGO_LCS_HPP


#include <vector>
#include <algorithm>


namespace bpp
{
	/**
	 * Implements a longes common subsequence algorithm, which founds only the length of the LCS itself.
	 * \tparam RES The result type (must be an integral type).
	 * \tparam CONTAINER Class holding the sequence. The class must have size() method
	 *         and the comparator must be able to get values from the container based on their indices.
	 * \tparma COMPARATOR Comparator class holds a static method compare(seq1, i1, seq2, i2) -> bool.
	 *         I.e., the comparator is also responsible for fetching values from the seq. containers.
	 */
	template<typename RES = std::size_t, class CONTAINER, typename COMPARATOR>
	RES longest_common_subsequence_length(const CONTAINER &sequence1, const CONTAINER &sequence2, COMPARATOR comparator)
	{
		if (sequence1.size() == 0 || sequence2.size() == 0) return (RES)0;

		// Make sure in seq1 is the longer sequence ...
		const CONTAINER &seq1 = sequence1.size() >= sequence2.size() ? sequence1 : sequence2;
		const CONTAINER &seq2 = sequence1.size() < sequence2.size() ? sequence1 : sequence2;

		std::vector<RES> row((std::size_t)seq2.size());
		std::size_t rows = (std::size_t)seq1.size();

		// Dynamic programming - matrix traversal that keeps only the last row.
		for (std::size_t r = 0; r < rows; ++r) {
			RES lastUpperLeft = 0, lastLeft = 0;
			for (std::size_t i = 0; i < row.size(); ++i) {
				RES upper = row[i];
				row[i] =
					(comparator(seq1, r, seq2, i))
					? lastUpperLeft + 1
					: std::max(lastLeft, upper);
				lastLeft = row[i];
				lastUpperLeft = upper;
			}
		}

		return row.back();
	}


	// Only an overload that uses default comparator.
	template<typename RES = std::size_t, class CONTAINER>
	RES longest_common_subsequence_length(const CONTAINER &sequence1, const CONTAINER &sequence2)
	{
		return longest_common_subsequence_length<RES>(sequence1, sequence2,
			[](const CONTAINER &seq1, std::size_t i1, const CONTAINER &seq2, std::size_t i2) -> bool {
				return seq1[i1] == seq2[i2];
			}
		);
	}
}

#endif

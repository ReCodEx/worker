/*
 * Author: Martin Krulis <krulis@ksi.mff.cuni.cz>
 * Last Modification: 24.5.2018
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
	 * \tparam COMPARATOR Comparator class holds a static method compare(seq1, i1, seq2, i2) -> bool.
	 *         I.e., the comparator is also responsible for fetching values from the seq. containers.
	 */
	template <typename RES = std::size_t, class CONTAINER, typename COMPARATOR>
	RES longest_common_subsequence_length(const CONTAINER &sequence1, const CONTAINER &sequence2, COMPARATOR comparator)
	{
		if (sequence1.size() == 0 || sequence2.size() == 0) return (RES) 0;

		// Make sure in seq1 is the longer sequence ...
		const CONTAINER &seq1 = sequence1.size() >= sequence2.size() ? sequence1 : sequence2;
		const CONTAINER &seq2 = sequence1.size() < sequence2.size() ? sequence1 : sequence2;

		std::vector<RES> row((std::size_t) seq2.size());
		auto rows = (std::size_t) seq1.size();

		// Dynamic programming - matrix traversal that keeps only the last row.
		for (std::size_t r = 0; r < rows; ++r) {
			RES lastUpperLeft = 0, lastLeft = 0;
			for (std::size_t i = 0; i < row.size(); ++i) {
				RES upper = row[i];
				row[i] = (comparator(seq1, r, seq2, i)) ? lastUpperLeft + 1 : std::max(lastLeft, upper);
				lastLeft = row[i];
				lastUpperLeft = upper;
			}
		}

		return row.back();
	}


	// Only an overload that uses default comparator.
	template <typename RES = std::size_t, class CONTAINER>
	RES longest_common_subsequence_length(const CONTAINER &sequence1, const CONTAINER &sequence2)
	{
		return longest_common_subsequence_length<RES>(sequence1,
			sequence2,
			[](const CONTAINER &seq1, std::size_t i1, const CONTAINER &seq2, std::size_t i2) -> bool {
				return seq1[i1] == seq2[i2];
			});
	}


	/**
	 * Implements a longes common subsequence algorithm, which founds only the length of the LCS itself.
	 * \tparam RES The result type (must be an integral type).
	 * \tparam CONTAINER Class holding the sequence. The class must have size() method
	 *         and the comparator must be able to get values from the container based on their indices.
	 * \tparam COMPARATOR Comparator class holds a static method compare(seq1, i1, seq2, i2) -> bool.
	 *         I.e., the comparator is also responsible for fetching values from the seq. containers.
	 */
	template <typename IDX = std::size_t, class CONTAINER, typename COMPARATOR>
	void longest_common_subsequence(const CONTAINER &sequence1,
		const CONTAINER &sequence2,
		std::vector<std::pair<IDX, IDX>> &common,
		COMPARATOR comparator)
	{
		struct Node {
			std::pair<IDX, IDX> previous;
			IDX length;
			bool match;
		};

		common.clear();
		if (sequence1.size() == 0 || sequence2.size() == 0) return;

		const std::size_t size1 = sequence1.size();
		const std::size_t size2 = sequence2.size();

		// Prepare vector representing the LCS matrix ...
		std::vector<Node> matrix((size1 + 1) * (size2 + 1));
		for (std::size_t i = 0; i < size1; ++i) { matrix[i + 1].previous.first = 1; }
		for (std::size_t i = 0; i < size2; ++i) { matrix[(i + 1) * (size1 + 1)].previous.second = 1; }

		// Fill in the LCS matrix by dynamic programming
		std::size_t i = size1 + 2; // current position in matrix (i == (c+1)*(size1+1) + (r+1))
		for (std::size_t r = 0; r < size2; ++r) { // iterate over rows
			for (std::size_t c = 0; c < size1; ++c) { // iterate over cols
				matrix[i].match = comparator(sequence1, c, sequence2, r);

				if (matrix[i].match) {
					// Matching tokens should prolong the sequence...
					matrix[i].length = matrix[i - size1 - 2].length + 1;
					matrix[i].previous.first = 1;
					matrix[i].previous.second = 1;
				} else {
					IDX leftLength = matrix[i - 1].length;
					IDX upperLength = matrix[i - size1 - 1].length;
					if (leftLength >= upperLength) {
						matrix[i].previous.first = 1;
						matrix[i].length = leftLength;
					} else {
						matrix[i].previous.second = 1;
						matrix[i].length = upperLength;
					}
				}
				++i;
			}
			++i; // skip the first (padding) column
		}

		// Collect the result path from the matrix...
		std::size_t c = size1;
		std::size_t r = size2;
		while (c > 0 && r > 0) {
			const Node &node = matrix[r * (size1 + 1) + c];
			if (node.match) { common.push_back(std::make_pair<IDX, IDX>(c - 1, r - 1)); }

			c -= node.previous.first;
			r -= node.previous.second;
		}

		// Fix the result (since it was callected backwards)...
		std::reverse(common.begin(), common.end());
	}


	// Only an overload that uses default comparator.
	template <typename IDX = std::size_t, class CONTAINER>
	void longest_common_subsequence(
		const CONTAINER &sequence1, const CONTAINER &sequence2, std::vector<std::pair<IDX, IDX>> &common)
	{
		longest_common_subsequence<IDX>(sequence1,
			sequence2,
			common,
			[](const CONTAINER &seq1, std::size_t i1, const CONTAINER &seq2, std::size_t i2) -> bool {
				return seq1[i1] == seq2[i2];
			});
	}


} // namespace bpp

#endif

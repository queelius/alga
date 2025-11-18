#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cmath>

namespace alga {
namespace similarity {

/**
 * @brief Levenshtein distance (edit distance)
 *
 * Minimum number of single-character edits (insertions, deletions, substitutions)
 * required to change one string into another.
 *
 * Complexity: O(m*n) time, O(min(m,n)) space
 */
inline size_t levenshtein_distance(std::string_view s1, std::string_view s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();

    if (m == 0) return n;
    if (n == 0) return m;

    // Use single array with size n+1
    std::vector<size_t> prev_row(n + 1);
    std::vector<size_t> curr_row(n + 1);

    // Initialize first row
    for (size_t j = 0; j <= n; ++j) {
        prev_row[j] = j;
    }

    // Fill matrix
    for (size_t i = 1; i <= m; ++i) {
        curr_row[0] = i;

        for (size_t j = 1; j <= n; ++j) {
            size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

            curr_row[j] = std::min({
                prev_row[j] + 1,      // deletion
                curr_row[j - 1] + 1,  // insertion
                prev_row[j - 1] + cost // substitution
            });
        }

        std::swap(prev_row, curr_row);
    }

    return prev_row[n];
}

/**
 * @brief Normalized Levenshtein similarity (0-1 range)
 *
 * Returns 1.0 for identical strings, 0.0 for completely different.
 */
inline double levenshtein_similarity(std::string_view s1, std::string_view s2) {
    size_t max_len = std::max(s1.size(), s2.size());
    if (max_len == 0) return 1.0;

    size_t distance = levenshtein_distance(s1, s2);
    return 1.0 - (static_cast<double>(distance) / max_len);
}

/**
 * @brief Hamming distance
 *
 * Number of positions at which corresponding symbols differ.
 * Only defined for strings of equal length.
 *
 * Returns nullopt if strings have different lengths.
 */
inline std::optional<size_t> hamming_distance(std::string_view s1, std::string_view s2) {
    if (s1.size() != s2.size()) {
        return std::nullopt;
    }

    size_t distance = 0;
    for (size_t i = 0; i < s1.size(); ++i) {
        if (s1[i] != s2[i]) {
            ++distance;
        }
    }

    return distance;
}

/**
 * @brief Jaro similarity
 *
 * Measures similarity between two strings, range [0, 1].
 * 0 = no similarity, 1 = identical
 *
 * Good for short strings like names.
 */
inline double jaro_similarity(std::string_view s1, std::string_view s2) {
    const size_t len1 = s1.size();
    const size_t len2 = s2.size();

    if (len1 == 0 && len2 == 0) return 1.0;
    if (len1 == 0 || len2 == 0) return 0.0;

    // Maximum allowed distance for matching
    size_t match_distance = std::max(len1, len2) / 2;
    if (match_distance > 0) --match_distance;

    std::vector<bool> s1_matches(len1, false);
    std::vector<bool> s2_matches(len2, false);

    size_t matches = 0;
    size_t transpositions = 0;

    // Identify matches
    for (size_t i = 0; i < len1; ++i) {
        size_t start = (i > match_distance) ? i - match_distance : 0;
        size_t end = std::min(i + match_distance + 1, len2);

        for (size_t j = start; j < end; ++j) {
            if (s2_matches[j] || s1[i] != s2[j]) continue;
            s1_matches[i] = true;
            s2_matches[j] = true;
            ++matches;
            break;
        }
    }

    if (matches == 0) return 0.0;

    // Count transpositions
    size_t k = 0;
    for (size_t i = 0; i < len1; ++i) {
        if (!s1_matches[i]) continue;
        while (!s2_matches[k]) ++k;
        if (s1[i] != s2[k]) ++transpositions;
        ++k;
    }

    double m = static_cast<double>(matches);
    return (m / len1 + m / len2 + (m - transpositions / 2.0) / m) / 3.0;
}

/**
 * @brief Jaro-Winkler similarity
 *
 * Extension of Jaro similarity that gives more weight to common prefixes.
 * Range [0, 1], where 1 = identical.
 *
 * Better than Jaro for strings with common prefixes (like names).
 */
inline double jaro_winkler_similarity(std::string_view s1, std::string_view s2, double prefix_scale = 0.1) {
    double jaro = jaro_similarity(s1, s2);

    if (jaro < 0.7) return jaro;  // Only boost if already similar

    // Find common prefix (up to 4 characters)
    size_t prefix_len = 0;
    size_t max_prefix = std::min({size_t(4), s1.size(), s2.size()});

    for (size_t i = 0; i < max_prefix; ++i) {
        if (s1[i] == s2[i]) {
            ++prefix_len;
        } else {
            break;
        }
    }

    return jaro + prefix_len * prefix_scale * (1.0 - jaro);
}

/**
 * @brief Longest Common Subsequence (LCS) length
 *
 * Length of longest subsequence common to both strings.
 * Note: subsequence doesn't need to be contiguous.
 */
inline size_t lcs_length(std::string_view s1, std::string_view s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();

    if (m == 0 || n == 0) return 0;

    // Use two rows for space efficiency
    std::vector<size_t> prev(n + 1, 0);
    std::vector<size_t> curr(n + 1, 0);

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                curr[j] = prev[j - 1] + 1;
            } else {
                curr[j] = std::max(prev[j], curr[j - 1]);
            }
        }
        std::swap(prev, curr);
    }

    return prev[n];
}

/**
 * @brief LCS-based similarity (0-1 range)
 *
 * Ratio of LCS length to average string length.
 */
inline double lcs_similarity(std::string_view s1, std::string_view s2) {
    if (s1.empty() && s2.empty()) return 1.0;
    if (s1.empty() || s2.empty()) return 0.0;

    size_t lcs = lcs_length(s1, s2);
    double avg_len = (s1.size() + s2.size()) / 2.0;

    return lcs / avg_len;
}

/**
 * @brief Damerau-Levenshtein distance
 *
 * Like Levenshtein but also allows transpositions (swapping adjacent characters).
 * More accurate for typos than plain Levenshtein.
 */
inline size_t damerau_levenshtein_distance(std::string_view s1, std::string_view s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();

    if (m == 0) return n;
    if (n == 0) return m;

    // Need full matrix for transpositions
    std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));

    for (size_t i = 0; i <= m; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j) dp[0][j] = j;

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

            dp[i][j] = std::min({
                dp[i - 1][j] + 1,       // deletion
                dp[i][j - 1] + 1,       // insertion
                dp[i - 1][j - 1] + cost // substitution
            });

            // Transposition
            if (i > 1 && j > 1 &&
                s1[i - 1] == s2[j - 2] &&
                s1[i - 2] == s2[j - 1]) {
                dp[i][j] = std::min(dp[i][j], dp[i - 2][j - 2] + cost);
            }
        }
    }

    return dp[m][n];
}

/**
 * @brief Check if two strings are "similar enough"
 *
 * Convenience function using Jaro-Winkler similarity.
 * Returns true if similarity >= threshold (default 0.8).
 */
inline bool are_similar(std::string_view s1, std::string_view s2, double threshold = 0.8) {
    return jaro_winkler_similarity(s1, s2) >= threshold;
}

/**
 * @brief Check if string is within edit distance N
 *
 * Returns true if Levenshtein distance <= max_distance.
 */
inline bool within_distance(std::string_view s1, std::string_view s2, size_t max_distance) {
    return levenshtein_distance(s1, s2) <= max_distance;
}

} // namespace similarity
} // namespace alga

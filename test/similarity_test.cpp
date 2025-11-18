/**
 * @file similarity_test.cpp
 * @brief Comprehensive tests for string similarity metrics
 */

#include <gtest/gtest.h>
#include "parsers/similarity.hpp"
#include <string>

using namespace alga;
using namespace alga::similarity;

// Helper for floating point comparison
bool approx_equal(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// ============================================================================
// Levenshtein Distance Tests
// ============================================================================

class LevenshteinTest : public ::testing::Test {};

TEST_F(LevenshteinTest, IdenticalStrings) {
    EXPECT_EQ(levenshtein_distance("hello", "hello"), 0UL);
    EXPECT_EQ(levenshtein_distance("test", "test"), 0UL);
}

TEST_F(LevenshteinTest, SingleInsertion) {
    EXPECT_EQ(levenshtein_distance("cat", "cats"), 1UL);
    EXPECT_EQ(levenshtein_distance("", "a"), 1UL);
}

TEST_F(LevenshteinTest, SingleDeletion) {
    EXPECT_EQ(levenshtein_distance("cats", "cat"), 1UL);
    EXPECT_EQ(levenshtein_distance("a", ""), 1UL);
}

TEST_F(LevenshteinTest, SingleSubstitution) {
    EXPECT_EQ(levenshtein_distance("cat", "bat"), 1UL);
    EXPECT_EQ(levenshtein_distance("hello", "hallo"), 1UL);
}

TEST_F(LevenshteinTest, MultipleEdits) {
    EXPECT_EQ(levenshtein_distance("kitten", "sitting"), 3UL);  // k→s, e→i, +g
    EXPECT_EQ(levenshtein_distance("saturday", "sunday"), 3UL);
}

TEST_F(LevenshteinTest, CompletelyDifferent) {
    EXPECT_EQ(levenshtein_distance("abc", "xyz"), 3UL);
}

TEST_F(LevenshteinTest, EmptyStrings) {
    EXPECT_EQ(levenshtein_distance("", ""), 0UL);
    EXPECT_EQ(levenshtein_distance("hello", ""), 5UL);
    EXPECT_EQ(levenshtein_distance("", "world"), 5UL);
}

TEST_F(LevenshteinTest, CaseSensitive) {
    EXPECT_EQ(levenshtein_distance("Hello", "hello"), 1UL);  // H→h
}

TEST_F(LevenshteinTest, Similarity) {
    EXPECT_DOUBLE_EQ(levenshtein_similarity("hello", "hello"), 1.0);
    EXPECT_DOUBLE_EQ(levenshtein_similarity("", ""), 1.0);
    EXPECT_GT(levenshtein_similarity("hello", "hallo"), 0.5);
    EXPECT_LT(levenshtein_similarity("abc", "xyz"), 0.5);
}

// ============================================================================
// Hamming Distance Tests
// ============================================================================

class HammingTest : public ::testing::Test {};

TEST_F(HammingTest, EqualLength) {
    auto dist = hamming_distance("hello", "hallo");
    ASSERT_TRUE(dist.has_value());
    EXPECT_EQ(*dist, 1UL);
}

TEST_F(HammingTest, IdenticalStrings) {
    auto dist = hamming_distance("test", "test");
    ASSERT_TRUE(dist.has_value());
    EXPECT_EQ(*dist, 0UL);
}

TEST_F(HammingTest, CompletelyDifferent) {
    auto dist = hamming_distance("abcd", "wxyz");
    ASSERT_TRUE(dist.has_value());
    EXPECT_EQ(*dist, 4UL);
}

TEST_F(HammingTest, DifferentLengths) {
    auto dist = hamming_distance("hello", "hi");
    EXPECT_FALSE(dist.has_value());
}

TEST_F(HammingTest, EmptyStrings) {
    auto dist = hamming_distance("", "");
    ASSERT_TRUE(dist.has_value());
    EXPECT_EQ(*dist, 0UL);
}

// ============================================================================
// Jaro Similarity Tests
// ============================================================================

class JaroTest : public ::testing::Test {};

TEST_F(JaroTest, IdenticalStrings) {
    EXPECT_DOUBLE_EQ(jaro_similarity("hello", "hello"), 1.0);
}

TEST_F(JaroTest, CompletelyDifferent) {
    EXPECT_DOUBLE_EQ(jaro_similarity("abc", "xyz"), 0.0);
}

TEST_F(JaroTest, SimilarNames) {
    double sim = jaro_similarity("MARTHA", "MARHTA");
    EXPECT_GT(sim, 0.9);  // High similarity (transposition)
}

TEST_F(JaroTest, DixonDixson) {
    double sim = jaro_similarity("DIXON", "DICKSONX");
    EXPECT_GT(sim, 0.7);
}

TEST_F(JaroTest, EmptyStrings) {
    EXPECT_DOUBLE_EQ(jaro_similarity("", ""), 1.0);
    EXPECT_DOUBLE_EQ(jaro_similarity("hello", ""), 0.0);
}

// ============================================================================
// Jaro-Winkler Tests
// ============================================================================

class JaroWinklerTest : public ::testing::Test {};

TEST_F(JaroWinklerTest, IdenticalStrings) {
    EXPECT_DOUBLE_EQ(jaro_winkler_similarity("test", "test"), 1.0);
}

TEST_F(JaroWinklerTest, CommonPrefix) {
    // Jaro-Winkler favors common prefixes
    double jw = jaro_winkler_similarity("hello", "hallo");
    double j = jaro_similarity("hello", "hallo");
    EXPECT_GT(jw, j);  // JW should be higher due to 'h' prefix
}

TEST_F(JaroWinklerTest, NoPrefix) {
    // No common prefix
    double sim = jaro_winkler_similarity("abc", "xyz");
    EXPECT_DOUBLE_EQ(sim, 0.0);
}

TEST_F(JaroWinklerTest, Names) {
    // Common use case: name matching
    double sim1 = jaro_winkler_similarity("Martha", "Marhta");
    double sim2 = jaro_winkler_similarity("Dwayne", "Duane");

    EXPECT_GT(sim1, 0.9);
    EXPECT_GT(sim2, 0.8);
}

// ============================================================================
// LCS Tests
// ============================================================================

class LCSTest : public ::testing::Test {};

TEST_F(LCSTest, IdenticalStrings) {
    EXPECT_EQ(lcs_length("hello", "hello"), 5UL);
}

TEST_F(LCSTest, CompletelyDifferent) {
    EXPECT_EQ(lcs_length("abc", "xyz"), 0UL);
}

TEST_F(LCSTest, CommonSubsequence) {
    EXPECT_EQ(lcs_length("ABCDGH", "AEDFHR"), 3UL);  // ADH
    EXPECT_EQ(lcs_length("AGGTAB", "GXTXAYB"), 4UL); // GTAB
}

TEST_F(LCSTest, OneEmpty) {
    EXPECT_EQ(lcs_length("", "hello"), 0UL);
    EXPECT_EQ(lcs_length("world", ""), 0UL);
}

TEST_F(LCSTest, Similarity) {
    EXPECT_DOUBLE_EQ(lcs_similarity("hello", "hello"), 1.0);
    EXPECT_DOUBLE_EQ(lcs_similarity("", ""), 1.0);
    EXPECT_GT(lcs_similarity("hello", "hallo"), 0.6);
}

// ============================================================================
// Damerau-Levenshtein Tests
// ============================================================================

class DamerauLevenshteinTest : public ::testing::Test {};

TEST_F(DamerauLevenshteinTest, Transposition) {
    // Damerau-Levenshtein handles transpositions better
    EXPECT_EQ(damerau_levenshtein_distance("ab", "ba"), 1UL);  // One transposition
    EXPECT_EQ(damerau_levenshtein_distance("hello", "ehllo"), 1UL);
}

TEST_F(DamerauLevenshteinTest, IdenticalStrings) {
    EXPECT_EQ(damerau_levenshtein_distance("test", "test"), 0UL);
}

TEST_F(DamerauLevenshteinTest, CommonTypos) {
    // Common keyboard typos
    EXPECT_EQ(damerau_levenshtein_distance("the", "teh"), 1UL);
    EXPECT_EQ(damerau_levenshtein_distance("form", "from"), 1UL);
}

TEST_F(DamerauLevenshteinTest, MultipleEdits) {
    EXPECT_EQ(damerau_levenshtein_distance("kitten", "sitting"), 3UL);
}

// ============================================================================
// Convenience Functions
// ============================================================================

TEST(SimilarityConvenience, AreSimilar) {
    EXPECT_TRUE(are_similar("hello", "hello"));
    EXPECT_TRUE(are_similar("hello", "hallo", 0.8));
    EXPECT_FALSE(are_similar("hello", "world", 0.8));
}

TEST(SimilarityConvenience, WithinDistance) {
    EXPECT_TRUE(within_distance("hello", "hallo", 1));
    EXPECT_TRUE(within_distance("hello", "hallo", 2));
    EXPECT_FALSE(within_distance("hello", "world", 2));
}

// ============================================================================
// Practical Use Cases
// ============================================================================

TEST(SimilarityUseCases, SpellChecking) {
    std::string correct = "definitely";
    std::vector<std::string> misspellings = {
        "definately",
        "definetly",
        "defiantly"
    };

    for (const auto& misspelling : misspellings) {
        // All should be within reasonable edit distance
        EXPECT_LE(levenshtein_distance(correct, misspelling), 4UL);
    }
}

TEST(SimilarityUseCases, NameMatching) {
    // Fuzzy name matching
    EXPECT_GT(jaro_winkler_similarity("Johnson", "Jonson"), 0.9);
    EXPECT_GT(jaro_winkler_similarity("Smith", "Smyth"), 0.85);
}

TEST(SimilarityUseCases, FindClosest) {
    std::vector<std::string> words = {"apple", "application", "apply", "banana"};
    std::string query = "aple";

    // Find closest match
    std::string closest;
    size_t min_distance = SIZE_MAX;

    for (const auto& word : words) {
        size_t dist = levenshtein_distance(query, word);
        if (dist < min_distance) {
            min_distance = dist;
            closest = word;
        }
    }

    EXPECT_EQ(closest, "apple");
    EXPECT_EQ(min_distance, 1UL);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(SimilarityEdgeCases, VeryLongStrings) {
    std::string long1(1000, 'a');
    std::string long2(1000, 'b');

    EXPECT_EQ(levenshtein_distance(long1, long1), 0UL);
    EXPECT_EQ(levenshtein_distance(long1, long2), 1000UL);
}

TEST(SimilarityEdgeCases, UnicodeHandling) {
    // Should work with UTF-8 (treats as byte sequences)
    std::string s1 = "café";
    std::string s2 = "cafe";

    // Distance will depend on byte representation
    size_t dist = levenshtein_distance(s1, s2);
    EXPECT_GT(dist, 0UL);  // Different due to é
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

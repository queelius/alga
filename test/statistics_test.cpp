/**
 * @file statistics_test.cpp
 * @brief Comprehensive tests for statistics module
 *
 * Tests frequency counting, entropy, diversity, and other statistical measures.
 */

#include <gtest/gtest.h>
#include "parsers/statistics.hpp"
#include <string>
#include <vector>
#include <cmath>

using namespace alga;
using namespace alga::statistics;

// Helper for floating point comparison
bool approx_equal(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// ============================================================================
// FrequencyCounter Tests
// ============================================================================

class FrequencyCounterTest : public ::testing::Test {};

TEST_F(FrequencyCounterTest, AddSingleElement) {
    FrequencyCounter<std::string> counter;
    counter.add("hello");

    EXPECT_EQ(counter.count("hello"), 1UL);
    EXPECT_EQ(counter.total(), 1UL);
    EXPECT_EQ(counter.unique_count(), 1UL);
}

TEST_F(FrequencyCounterTest, AddMultipleElements) {
    FrequencyCounter<std::string> counter;
    counter.add("hello");
    counter.add("world");
    counter.add("hello");

    EXPECT_EQ(counter.count("hello"), 2UL);
    EXPECT_EQ(counter.count("world"), 1UL);
    EXPECT_EQ(counter.total(), 3UL);
    EXPECT_EQ(counter.unique_count(), 2UL);
}

TEST_F(FrequencyCounterTest, AddAll) {
    FrequencyCounter<int> counter;
    std::vector<int> numbers = {1, 2, 3, 1, 2, 1};
    counter.add_all(numbers);

    EXPECT_EQ(counter.count(1), 3UL);
    EXPECT_EQ(counter.count(2), 2UL);
    EXPECT_EQ(counter.count(3), 1UL);
    EXPECT_EQ(counter.total(), 6UL);
}

TEST_F(FrequencyCounterTest, Frequency) {
    FrequencyCounter<char> counter;
    counter.add('a');
    counter.add('b');
    counter.add('a');
    counter.add('a');

    EXPECT_DOUBLE_EQ(counter.frequency('a'), 0.75);  // 3/4
    EXPECT_DOUBLE_EQ(counter.frequency('b'), 0.25);  // 1/4
    EXPECT_DOUBLE_EQ(counter.frequency('c'), 0.0);   // not present
}

TEST_F(FrequencyCounterTest, MostCommon) {
    FrequencyCounter<std::string> counter;
    counter.add("apple");
    counter.add("banana");
    counter.add("apple");
    counter.add("cherry");
    counter.add("apple");

    auto common = counter.most_common();
    ASSERT_EQ(common.size(), 3UL);
    EXPECT_EQ(common[0].first, "apple");
    EXPECT_EQ(common[0].second, 3UL);
}

TEST_F(FrequencyCounterTest, TopN) {
    FrequencyCounter<int> counter;
    for (int i = 1; i <= 10; ++i) {
        for (int j = 0; j < i; ++j) {
            counter.add(i);
        }
    }

    auto top3 = counter.top_n(3);
    ASSERT_EQ(top3.size(), 3UL);
    EXPECT_EQ(top3[0].first, 10);
    EXPECT_EQ(top3[1].first, 9);
    EXPECT_EQ(top3[2].first, 8);
}

TEST_F(FrequencyCounterTest, Mode) {
    FrequencyCounter<char> counter;
    counter.add('x');
    counter.add('y');
    counter.add('x');
    counter.add('x');

    auto mode = counter.mode();
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(*mode, 'x');
}

TEST_F(FrequencyCounterTest, EmptyCounter) {
    FrequencyCounter<int> counter;

    EXPECT_EQ(counter.total(), 0UL);
    EXPECT_EQ(counter.unique_count(), 0UL);
    EXPECT_FALSE(counter.mode().has_value());
}

TEST_F(FrequencyCounterTest, Clear) {
    FrequencyCounter<int> counter;
    counter.add(1);
    counter.add(2);

    counter.clear();

    EXPECT_EQ(counter.total(), 0UL);
    EXPECT_EQ(counter.unique_count(), 0UL);
}

// ============================================================================
// Shannon Entropy Tests
// ============================================================================

class EntropyTest : public ::testing::Test {};

TEST_F(EntropyTest, UniformDistribution) {
    FrequencyCounter<int> counter;
    // Uniform: all elements equally likely
    counter.add(1);
    counter.add(2);
    counter.add(3);
    counter.add(4);

    double entropy = shannon_entropy(counter);
    // log2(4) = 2.0 (maximum entropy for 4 elements)
    EXPECT_TRUE(approx_equal(entropy, 2.0));
}

TEST_F(EntropyTest, SingleElement) {
    FrequencyCounter<char> counter;
    counter.add('a');
    counter.add('a');
    counter.add('a');

    double entropy = shannon_entropy(counter);
    // All same element = zero entropy
    EXPECT_DOUBLE_EQ(entropy, 0.0);
}

TEST_F(EntropyTest, SkewedDistribution) {
    FrequencyCounter<int> counter;
    counter.add(1);
    counter.add(1);
    counter.add(1);
    counter.add(2);

    double entropy = shannon_entropy(counter);
    // Entropy should be < log2(2) = 1.0
    EXPECT_LT(entropy, 1.0);
    EXPECT_GT(entropy, 0.0);
}

TEST_F(EntropyTest, NormalizedEntropy) {
    FrequencyCounter<int> counter;
    // Uniform distribution
    for (int i = 0; i < 8; ++i) {
        counter.add(i);
    }

    double norm_entropy = normalized_entropy(counter);
    // Perfect uniform = normalized entropy of 1.0
    EXPECT_TRUE(approx_equal(norm_entropy, 1.0));
}

TEST_F(EntropyTest, NormalizedEntropySkewed) {
    FrequencyCounter<int> counter;
    counter.add(1);
    counter.add(1);
    counter.add(1);
    counter.add(2);

    double norm_entropy = normalized_entropy(counter);
    EXPECT_GT(norm_entropy, 0.0);
    EXPECT_LT(norm_entropy, 1.0);
}

// ============================================================================
// Diversity Tests
// ============================================================================

class DiversityTest : public ::testing::Test {};

TEST_F(DiversityTest, SimpsonDiversityUniform) {
    FrequencyCounter<int> counter;
    counter.add(1);
    counter.add(2);
    counter.add(3);
    counter.add(4);

    double diversity = simpson_diversity(counter);
    // Uniform distribution: 1 - 4*(0.25^2) = 0.75
    EXPECT_TRUE(approx_equal(diversity, 0.75));
}

TEST_F(DiversityTest, SimpsonDiversitySingleElement) {
    FrequencyCounter<char> counter;
    counter.add('a');
    counter.add('a');

    double diversity = simpson_diversity(counter);
    // All same = no diversity
    EXPECT_DOUBLE_EQ(diversity, 0.0);
}

TEST_F(DiversityTest, TypeTokenRatio) {
    FrequencyCounter<std::string> counter;
    counter.add("the");
    counter.add("cat");
    counter.add("sat");
    counter.add("on");
    counter.add("the");
    counter.add("mat");

    double ttr = type_token_ratio(counter);
    // 5 unique / 6 total = 0.833...
    EXPECT_TRUE(approx_equal(ttr, 5.0/6.0));
}

TEST_F(DiversityTest, TypeTokenRatioAllUnique) {
    FrequencyCounter<int> counter;
    for (int i = 0; i < 10; ++i) {
        counter.add(i);
    }

    double ttr = type_token_ratio(counter);
    EXPECT_DOUBLE_EQ(ttr, 1.0);  // All unique
}

// ============================================================================
// Numeric Summary Tests
// ============================================================================

class NumericSummaryTest : public ::testing::Test {};

TEST_F(NumericSummaryTest, BasicStats) {
    std::vector<int> data = {1, 2, 3, 4, 5};

    auto summary = numeric_summary(data);
    ASSERT_TRUE(summary.has_value());

    EXPECT_EQ(summary->count, 5UL);
    EXPECT_EQ(summary->min, 1);
    EXPECT_EQ(summary->max, 5);
    EXPECT_DOUBLE_EQ(summary->mean, 3.0);
    EXPECT_EQ(summary->median, 3);
}

TEST_F(NumericSummaryTest, Variance) {
    std::vector<int> data = {2, 4, 6, 8};

    auto summary = numeric_summary(data);
    ASSERT_TRUE(summary.has_value());

    EXPECT_DOUBLE_EQ(summary->mean, 5.0);
    // Variance = ((9 + 1 + 1 + 9) / 4) = 5.0
    EXPECT_DOUBLE_EQ(summary->variance, 5.0);
    EXPECT_TRUE(approx_equal(summary->std_dev, std::sqrt(5.0)));
}

TEST_F(NumericSummaryTest, MedianEvenCount) {
    std::vector<int> data = {1, 2, 3, 4};

    auto summary = numeric_summary(data);
    ASSERT_TRUE(summary.has_value());

    // Median of even count = average of middle two
    EXPECT_EQ(summary->median, 2);  // (2+3)/2 = 2 (integer division)
}

TEST_F(NumericSummaryTest, EmptyData) {
    std::vector<int> data;

    auto summary = numeric_summary(data);
    EXPECT_FALSE(summary.has_value());
}

TEST_F(NumericSummaryTest, SingleValue) {
    std::vector<double> data = {42.0};

    auto summary = numeric_summary(data);
    ASSERT_TRUE(summary.has_value());

    EXPECT_EQ(summary->count, 1UL);
    EXPECT_DOUBLE_EQ(summary->min, 42.0);
    EXPECT_DOUBLE_EQ(summary->max, 42.0);
    EXPECT_DOUBLE_EQ(summary->mean, 42.0);
    EXPECT_DOUBLE_EQ(summary->median, 42.0);
    EXPECT_DOUBLE_EQ(summary->variance, 0.0);
}

// ============================================================================
// Gini Coefficient Tests
// ============================================================================

class GiniTest : public ::testing::Test {};

TEST_F(GiniTest, PerfectEquality) {
    FrequencyCounter<int> counter;
    // All elements appear equally
    counter.add(1);
    counter.add(2);
    counter.add(3);
    counter.add(4);

    double gini = gini_coefficient(counter);
    // Perfect equality = Gini close to 0
    EXPECT_LT(gini, 0.3);  // Should be low
}

TEST_F(GiniTest, HighInequality) {
    FrequencyCounter<int> counter;
    // One element dominates
    for (int i = 0; i < 100; ++i) {
        counter.add(1);
    }
    counter.add(2);
    counter.add(3);

    double gini = gini_coefficient(counter);
    // High inequality = Gini close to 1
    EXPECT_GT(gini, 0.5);
}

// ============================================================================
// Hapax/Dis Legomena Tests
// ============================================================================

class LegomenaTest : public ::testing::Test {};

TEST_F(LegomenaTest, HapaxCount) {
    FrequencyCounter<std::string> counter;
    counter.add("rare1");        // appears once
    counter.add("common");
    counter.add("common");
    counter.add("rare2");        // appears once

    size_t hapax = hapax_legomena_count(counter);
    EXPECT_EQ(hapax, 2UL);
}

TEST_F(LegomenaTest, DisCount) {
    FrequencyCounter<char> counter;
    counter.add('a');
    counter.add('a');            // appears twice
    counter.add('b');
    counter.add('b');            // appears twice
    counter.add('c');
    counter.add('c');
    counter.add('c');

    size_t dis = dis_legomena_count(counter);
    EXPECT_EQ(dis, 2UL);  // 'a' and 'b'
}

// ============================================================================
// Full Analysis Tests
// ============================================================================

class AnalysisTest : public ::testing::Test {};

TEST_F(AnalysisTest, ComprehensiveAnalysis) {
    FrequencyCounter<std::string> counter;
    std::vector<std::string> words = {
        "the", "cat", "sat", "on", "the", "mat",
        "the", "cat", "was", "fat"
    };
    counter.add_all(words);

    auto analysis = analyze_distribution(counter);

    EXPECT_EQ(analysis.total_elements, 10UL);
    EXPECT_EQ(analysis.unique_elements, 7UL);
    EXPECT_GT(analysis.shannon_entropy, 0.0);
    EXPECT_GT(analysis.simpson_diversity, 0.0);
    EXPECT_GT(analysis.type_token_ratio, 0.0);
    EXPECT_LE(analysis.type_token_ratio, 1.0);
    EXPECT_GE(analysis.top_10.size(), 1UL);
}

TEST_F(AnalysisTest, TopElementsCorrect) {
    FrequencyCounter<int> counter;
    counter.add(1);
    counter.add(1);
    counter.add(1);
    counter.add(2);
    counter.add(2);
    counter.add(3);

    auto analysis = analyze_distribution(counter);

    ASSERT_GE(analysis.top_10.size(), 3UL);
    EXPECT_EQ(analysis.top_10[0].first, 1);
    EXPECT_EQ(analysis.top_10[0].second, 3UL);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(StatisticsIntegration, MakeFrequencyCounter) {
    std::vector<char> letters = {'a', 'b', 'c', 'a', 'b', 'a'};
    auto counter = make_frequency_counter(letters);

    EXPECT_EQ(counter.total(), 6UL);
    EXPECT_EQ(counter.count('a'), 3UL);
    EXPECT_EQ(counter.count('b'), 2UL);
    EXPECT_EQ(counter.count('c'), 1UL);
}

TEST(StatisticsIntegration, EntropyAndDiversity) {
    std::vector<int> data = {1, 1, 2, 2, 3, 3, 4, 4};
    auto counter = make_frequency_counter(data);

    double entropy = shannon_entropy(counter);
    double diversity = simpson_diversity(counter);

    // Both should indicate good diversity
    EXPECT_GT(entropy, 1.0);
    EXPECT_GT(diversity, 0.5);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(StatisticsEdgeCases, EmptyCounter) {
    FrequencyCounter<int> counter;

    EXPECT_DOUBLE_EQ(shannon_entropy(counter), 0.0);
    EXPECT_DOUBLE_EQ(simpson_diversity(counter), 0.0);
    EXPECT_DOUBLE_EQ(gini_coefficient(counter), 0.0);
    EXPECT_DOUBLE_EQ(type_token_ratio(counter), 0.0);
}

TEST(StatisticsEdgeCases, LargeDataset) {
    FrequencyCounter<int> counter;
    for (int i = 0; i < 10000; ++i) {
        counter.add(i % 100);  // 100 unique values, each appearing 100 times
    }

    EXPECT_EQ(counter.total(), 10000UL);
    EXPECT_EQ(counter.unique_count(), 100UL);

    double entropy = shannon_entropy(counter);
    // Uniform over 100 elements: log2(100) ≈ 6.64
    EXPECT_TRUE(approx_equal(entropy, std::log2(100), 0.01));
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

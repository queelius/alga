/**
 * @file count_combinators_test.cpp
 * @brief Comprehensive tests for count and range combinators
 *
 * Tests count, between, atLeast, atMost, times, and exactly combinators.
 */

#include <gtest/gtest.h>
#include "parsers/count_combinators.hpp"
#include <string>
#include <vector>
#include <optional>

using namespace alga;
using namespace alga::combinators;

// ============================================================================
// Helper Parsers (same as optional_combinator_test)
// ============================================================================

class DigitParser {
public:
    using output_type = char;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<char>>
    {
        if (begin == end || !std::isdigit(*begin)) {
            return {begin, std::nullopt};
        }
        return {begin + 1, std::make_optional(*begin)};
    }
};

inline auto digit_parser() {
    return DigitParser{};
}

class LetterParser {
public:
    using output_type = char;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<char>>
    {
        if (begin == end || !std::isalpha(*begin)) {
            return {begin, std::nullopt};
        }
        return {begin + 1, std::make_optional(*begin)};
    }
};

inline auto letter_parser() {
    return LetterParser{};
}

// ============================================================================
// Count Combinator Tests
// ============================================================================

class CountTest : public ::testing::Test {};

TEST_F(CountTest, ExactMatchSucceeds) {
    auto parser = count(3, digit_parser());
    std::string input = "123abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0], '1');
    EXPECT_EQ((*result)[1], '2');
    EXPECT_EQ((*result)[2], '3');
    EXPECT_EQ(std::string(pos, input.end()), "abc");
}

TEST_F(CountTest, TooFewMatchesFails) {
    auto parser = count(5, digit_parser());
    std::string input = "123abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(pos, input.begin());  // No input consumed on failure
}

TEST_F(CountTest, MoreThanNeededAvailable) {
    auto parser = count(3, digit_parser());
    std::string input = "12345678";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 3UL);  // Only takes exactly 3
    EXPECT_EQ(std::string(pos, input.end()), "45678");
}

TEST_F(CountTest, CountZero) {
    auto parser = count(0, digit_parser());
    std::string input = "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
    EXPECT_EQ(pos, input.begin());  // No input consumed
}

TEST_F(CountTest, EmptyInput) {
    auto parser = count(3, digit_parser());
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(CountTest, CountOne) {
    auto parser = count(1, letter_parser());
    std::string input = "a123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1UL);
    EXPECT_EQ((*result)[0], 'a');
}

// ============================================================================
// Between Combinator Tests
// ============================================================================

class BetweenTest : public ::testing::Test {};

TEST_F(BetweenTest, MinimumMatches) {
    auto parser = between(2, 5, digit_parser());
    std::string input = "12abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2UL);
    EXPECT_EQ((*result)[0], '1');
    EXPECT_EQ((*result)[1], '2');
}

TEST_F(BetweenTest, MiddleRange) {
    auto parser = between(2, 5, digit_parser());
    std::string input = "1234abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 4UL);
}

TEST_F(BetweenTest, MaximumMatches) {
    auto parser = between(2, 5, digit_parser());
    std::string input = "12345abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 5UL);  // Stops at maximum
    EXPECT_EQ(std::string(pos, input.end()), "abc");
}

TEST_F(BetweenTest, MoreThanMaximumAvailable) {
    auto parser = between(2, 4, digit_parser());
    std::string input = "123456789";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 4UL);  // Stops at maximum (greedy)
    EXPECT_EQ(std::string(pos, input.end()), "56789");
}

TEST_F(BetweenTest, LessThanMinimumFails) {
    auto parser = between(3, 5, digit_parser());
    std::string input = "12abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(pos, input.begin());
}

TEST_F(BetweenTest, EmptyInput) {
    auto parser = between(1, 3, digit_parser());
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(BetweenTest, MinEqualsMax) {
    auto parser = between(3, 3, digit_parser());
    std::string input = "12345";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 3UL);  // Exactly 3
}

TEST_F(BetweenTest, ZeroMin) {
    auto parser = between(0, 3, digit_parser());
    std::string input = "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());  // Zero is valid
}

// ============================================================================
// AtLeast Combinator Tests
// ============================================================================

class AtLeastTest : public ::testing::Test {};

TEST_F(AtLeastTest, ExactlyMinimum) {
    auto parser = atLeast(3, digit_parser());
    std::string input = "123abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 3UL);
}

TEST_F(AtLeastTest, MoreThanMinimum) {
    auto parser = atLeast(2, digit_parser());
    std::string input = "12345abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 5UL);  // Greedy, takes all available
}

TEST_F(AtLeastTest, LessThanMinimumFails) {
    auto parser = atLeast(5, digit_parser());
    std::string input = "123abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(AtLeastTest, ZeroMinimum) {
    auto parser = atLeast(0, digit_parser());
    std::string input = "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());  // Zero is valid minimum
}

TEST_F(AtLeastTest, AllInput) {
    auto parser = atLeast(1, digit_parser());
    std::string input = "987654321";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 9UL);
    EXPECT_EQ(pos, input.end());
}

TEST_F(AtLeastTest, EmptyInput) {
    auto parser = atLeast(1, digit_parser());
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// AtMost Combinator Tests
// ============================================================================

class AtMostTest : public ::testing::Test {};

TEST_F(AtMostTest, ZeroMatches) {
    auto parser = atMost(3, digit_parser());
    std::string input = "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST_F(AtMostTest, LessThanMaximum) {
    auto parser = atMost(5, digit_parser());
    std::string input = "123abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 3UL);
}

TEST_F(AtMostTest, ExactlyMaximum) {
    auto parser = atMost(5, digit_parser());
    std::string input = "12345abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 5UL);
}

TEST_F(AtMostTest, MoreThanMaximumAvailable) {
    auto parser = atMost(3, digit_parser());
    std::string input = "123456789";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 3UL);  // Stops at maximum
    EXPECT_EQ(std::string(pos, input.end()), "456789");
}

TEST_F(AtMostTest, EmptyInput) {
    auto parser = atMost(3, digit_parser());
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());  // Always succeeds
}

TEST_F(AtMostTest, ZeroMaximum) {
    auto parser = atMost(0, digit_parser());
    std::string input = "123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
    EXPECT_EQ(pos, input.begin());  // No input consumed
}

// ============================================================================
// Alias Tests
// ============================================================================

TEST(AliasTest, Times) {
    auto parser = times(4, digit_parser());
    std::string input = "1234abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 4UL);
}

TEST(AliasTest, Exactly) {
    auto parser = exactly(3, letter_parser());
    std::string input = "abc123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 3UL);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(CountCombinatorIntegration, CountFollowedByBetween) {
    auto prefix = count(2, letter_parser());
    auto suffix = between(1, 3, digit_parser());

    std::string input = "ab123";

    auto [pos1, result1] = prefix.parse(input.begin(), input.end());
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size(), 2UL);

    auto [pos2, result2] = suffix.parse(pos1, input.end());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2->size(), 3UL);
}

TEST(CountCombinatorIntegration, AtLeastFollowedByAtMost) {
    auto parser1 = atLeast(1, letter_parser());
    auto parser2 = atMost(2, digit_parser());

    std::string input = "hello12345";

    auto [pos1, result1] = parser1.parse(input.begin(), input.end());
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size(), 5UL);

    auto [pos2, result2] = parser2.parse(pos1, input.end());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2->size(), 2UL);  // Only takes first 2
}

TEST(CountCombinatorIntegration, NestedCounts) {
    // This doesn't really make sense semantically but tests composition
    auto parser = count(2, digit_parser());
    std::string input = "1234";

    auto [pos1, result1] = parser.parse(input.begin(), input.end());
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size(), 2UL);

    auto [pos2, result2] = parser.parse(pos1, input.end());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2->size(), 2UL);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(CountCombinatorEdgeCases, LargeCount) {
    auto parser = count(1000, digit_parser());
    std::string input(1000, '5');

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 1000UL);
}

TEST(CountCombinatorEdgeCases, BetweenWithLargeRange) {
    auto parser = between(1, 10000, digit_parser());
    std::string input(5000, '7');
    input += "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 5000UL);
}

TEST(CountCombinatorEdgeCases, AtLeastVeryLarge) {
    auto parser = atLeast(100, digit_parser());
    std::string input(500, '3');

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 500UL);
}

TEST(CountCombinatorEdgeCases, CountOneVersusPlainParser) {
    auto count_one = count(1, digit_parser());
    auto plain = digit_parser();

    std::string input = "5abc";

    auto [pos1, result1] = count_one.parse(input.begin(), input.end());
    auto [pos2, result2] = plain.parse(input.begin(), input.end());

    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result1->size(), 1UL);
    EXPECT_EQ((*result1)[0], *result2);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

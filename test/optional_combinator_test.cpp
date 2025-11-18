/**
 * @file optional_combinator_test.cpp
 * @brief Comprehensive tests for optional combinator and related parsers
 *
 * Tests optional, many, many1, and skip combinators.
 */

#include <gtest/gtest.h>
#include "parsers/optional_combinator.hpp"
#include <string>
#include <vector>
#include <optional>

using namespace alga;
using namespace alga::combinators;

// ============================================================================
// Helper Parsers
// ============================================================================

/**
 * @brief Simple digit parser (0-9)
 */
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

    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

inline auto digit_parser() {
    return DigitParser{};
}

/**
 * @brief Simple integer parser
 */
class IntParser {
public:
    using output_type = int;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<int>>
    {
        if (begin == end || !std::isdigit(*begin)) {
            return {begin, std::nullopt};
        }

        int result = 0;
        Iterator current = begin;

        while (current != end && std::isdigit(*current)) {
            result = result * 10 + (*current - '0');
            ++current;
        }

        return {current, std::make_optional(result)};
    }

    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

inline auto int_parser() {
    return IntParser{};
}

/**
 * @brief Letter parser (a-z, A-Z)
 */
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

    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

inline auto letter_parser() {
    return LetterParser{};
}

// ============================================================================
// Optional Combinator Tests
// ============================================================================

class OptionalTest : public ::testing::Test {};

TEST_F(OptionalTest, SucceedsWhenInnerParserSucceeds) {
    auto parser = optional(digit_parser());
    std::string input = "5abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());  // Outer optional (parser always succeeds)
    ASSERT_TRUE(result->has_value()); // Inner optional (digit was found)
    EXPECT_EQ(**result, '5');
    EXPECT_EQ(std::string(pos, input.end()), "abc");
}

TEST_F(OptionalTest, SucceedsWhenInnerParserFails) {
    auto parser = optional(digit_parser());
    std::string input = "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());   // Outer optional (parser always succeeds)
    EXPECT_FALSE(result->has_value()); // Inner optional (no digit found)
    EXPECT_EQ(pos, input.begin());     // No input consumed
}

TEST_F(OptionalTest, NoInputConsumedOnFailure) {
    auto parser = optional(int_parser());
    std::string input = "xyz123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->has_value());
    EXPECT_EQ(pos, input.begin());  // Position unchanged
}

TEST_F(OptionalTest, EmptyInput) {
    auto parser = optional(digit_parser());
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->has_value());
}

TEST_F(OptionalTest, WithIntParser) {
    auto parser = optional(int_parser());
    std::string input = "42xyz";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->has_value());
    EXPECT_EQ(**result, 42);
    EXPECT_EQ(std::string(pos, input.end()), "xyz");
}

// ============================================================================
// Many Combinator Tests
// ============================================================================

class ManyTest : public ::testing::Test {};

TEST_F(ManyTest, ZeroMatches) {
    auto parser = many(digit_parser());
    std::string input = "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());  // Empty vector
    EXPECT_EQ(pos, input.begin()); // No input consumed
}

TEST_F(ManyTest, OneMatch) {
    auto parser = many(digit_parser());
    std::string input = "5abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1UL);
    EXPECT_EQ((*result)[0], '5');
    EXPECT_EQ(std::string(pos, input.end()), "abc");
}

TEST_F(ManyTest, MultipleMatches) {
    auto parser = many(digit_parser());
    std::string input = "12345abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 5UL);
    EXPECT_EQ((*result)[0], '1');
    EXPECT_EQ((*result)[1], '2');
    EXPECT_EQ((*result)[2], '3');
    EXPECT_EQ((*result)[3], '4');
    EXPECT_EQ((*result)[4], '5');
    EXPECT_EQ(std::string(pos, input.end()), "abc");
}

TEST_F(ManyTest, AllInput) {
    auto parser = many(digit_parser());
    std::string input = "987654321";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 9UL);
    EXPECT_EQ(pos, input.end());
}

TEST_F(ManyTest, EmptyInput) {
    auto parser = many(digit_parser());
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST_F(ManyTest, WithLetterParser) {
    auto parser = many(letter_parser());
    std::string input = "hello123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 5UL);
    EXPECT_EQ((*result)[0], 'h');
    EXPECT_EQ((*result)[1], 'e');
    EXPECT_EQ((*result)[2], 'l');
    EXPECT_EQ((*result)[3], 'l');
    EXPECT_EQ((*result)[4], 'o');
}

// ============================================================================
// Many1 Combinator Tests
// ============================================================================

class Many1Test : public ::testing::Test {};

TEST_F(Many1Test, FailsOnZeroMatches) {
    auto parser = many1(digit_parser());
    std::string input = "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());  // Fails if no matches
    EXPECT_EQ(pos, input.begin());
}

TEST_F(Many1Test, SucceedsOnOneMatch) {
    auto parser = many1(digit_parser());
    std::string input = "7xyz";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1UL);
    EXPECT_EQ((*result)[0], '7');
}

TEST_F(Many1Test, SucceedsOnMultipleMatches) {
    auto parser = many1(digit_parser());
    std::string input = "98765abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 5UL);
    EXPECT_EQ((*result)[0], '9');
    EXPECT_EQ((*result)[4], '5');
}

TEST_F(Many1Test, EmptyInputFails) {
    auto parser = many1(digit_parser());
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(Many1Test, AllInput) {
    auto parser = many1(letter_parser());
    std::string input = "abcdefg";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 7UL);
    EXPECT_EQ(pos, input.end());
}

// ============================================================================
// Skip Combinator Tests
// ============================================================================

class SkipTest : public ::testing::Test {};

TEST_F(SkipTest, SucceedsAndAdvancesPosition) {
    auto parser = skip(digit_parser());
    std::string input = "5abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);  // Boolean true
    EXPECT_EQ(std::string(pos, input.end()), "abc");
}

TEST_F(SkipTest, FailsWhenInnerParserFails) {
    auto parser = skip(digit_parser());
    std::string input = "abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(pos, input.begin());
}

TEST_F(SkipTest, DiscardsResult) {
    auto parser = skip(int_parser());
    std::string input = "12345xyz";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);  // Just boolean, not the actual integer
    EXPECT_EQ(std::string(pos, input.end()), "xyz");
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(OptionalCombinatorIntegration, OptionalWithMany) {
    // Optional sign followed by many digits
    auto sign_parser = optional(letter_parser());
    std::string input = "a123";

    auto [sign_pos, sign_result] = sign_parser.parse(input.begin(), input.end());
    ASSERT_TRUE(sign_result.has_value());
    ASSERT_TRUE(sign_result->has_value());
    EXPECT_EQ(**sign_result, 'a');

    auto digits = many(digit_parser());
    auto [digit_pos, digit_result] = digits.parse(sign_pos, input.end());
    ASSERT_TRUE(digit_result.has_value());
    EXPECT_EQ(digit_result->size(), 3UL);
}

TEST(OptionalCombinatorIntegration, ManyOfOptional) {
    // This tests parsing a sequence where each element might be optional
    auto parser = many(optional(digit_parser()));
    std::string input = "123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    // Each successful parse returns optional<optional<char>>
    // The outer many collects these
}

TEST(OptionalCombinatorIntegration, Many1WithSkip) {
    // Parse one or more letters, then skip digits
    auto letters = many1(letter_parser());
    auto skip_digits = skip(many(digit_parser()));

    std::string input = "hello123";

    auto [pos1, result1] = letters.parse(input.begin(), input.end());
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size(), 5UL);

    auto [pos2, result2] = skip_digits.parse(pos1, input.end());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(pos2, input.end());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(OptionalCombinatorEdgeCases, NestedOptionals) {
    auto parser = optional(optional(digit_parser()));
    std::string input = "5";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    // Result is optional<optional<optional<char>>>
}

TEST(OptionalCombinatorEdgeCases, VeryLongSequence) {
    auto parser = many(digit_parser());
    std::string input(10000, '7');

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 10000UL);
}

TEST(OptionalCombinatorEdgeCases, AlternatingSuccess) {
    auto parser = many(digit_parser());
    std::string input = "1a2b3c4d";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1UL);  // Only first digit
    EXPECT_EQ((*result)[0], '1');
    EXPECT_EQ(std::string(pos, input.end()), "a2b3c4d");
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

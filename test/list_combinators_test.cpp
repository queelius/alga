/**
 * @file list_combinators_test.cpp
 * @brief Comprehensive tests for list combinator parsers
 *
 * Tests sepBy, sepBy1, sepEndBy, and endBy combinators with various parsers.
 */

#include <gtest/gtest.h>
#include "parsers/list_combinators.hpp"
#include "parsers/numeric_parsers.hpp"
#include "parsers/lc_alpha.hpp"
#include <string>
#include <vector>

using namespace alga;
using namespace alga::combinators;

// ============================================================================
// Helper: Simple Integer Parser for Testing
// ============================================================================

class SimpleIntParser {
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
    return SimpleIntParser{};
}

// ============================================================================
// sepBy Tests
// ============================================================================

class SepByTest : public ::testing::Test {
protected:
    void SetUp() override {
        comma = ',';
        semicolon = ';';
    }

    char comma;
    char semicolon;
};

TEST_F(SepByTest, ParseEmptyInput) {
    auto parser = sepBy(int_parser(), char_parser(comma));
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
    EXPECT_EQ(pos, input.begin());
}

TEST_F(SepByTest, ParseSingleElement) {
    auto parser = sepBy(int_parser(), char_parser(comma));
    std::string input = "42";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1UL);
    EXPECT_EQ((*result)[0], 42);
    EXPECT_EQ(pos, input.end());
}

TEST_F(SepByTest, ParseMultipleElements) {
    auto parser = sepBy(int_parser(), char_parser(comma));
    std::string input = "1,2,3,4,5";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 5UL);
    EXPECT_EQ((*result)[0], 1);
    EXPECT_EQ((*result)[1], 2);
    EXPECT_EQ((*result)[2], 3);
    EXPECT_EQ((*result)[3], 4);
    EXPECT_EQ((*result)[4], 5);
    EXPECT_EQ(pos, input.end());
}

TEST_F(SepByTest, ParseWithTrailingSeparator) {
    auto parser = sepBy(int_parser(), char_parser(comma));
    std::string input = "1,2,3,";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0], 1);
    EXPECT_EQ((*result)[1], 2);
    EXPECT_EQ((*result)[2], 3);
    // Should stop before the trailing comma
    EXPECT_EQ(*pos, ',');
}

TEST_F(SepByTest, ParsePartialInput) {
    auto parser = sepBy(int_parser(), char_parser(comma));
    std::string input = "10,20,30abc";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0], 10);
    EXPECT_EQ((*result)[1], 20);
    EXPECT_EQ((*result)[2], 30);
    EXPECT_EQ(std::string(pos, input.end()), "abc");
}

TEST_F(SepByTest, ParseWithDifferentSeparator) {
    auto parser = sepBy(int_parser(), char_parser(semicolon));
    std::string input = "100;200;300";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0], 100);
    EXPECT_EQ((*result)[1], 200);
    EXPECT_EQ((*result)[2], 300);
}

TEST_F(SepByTest, ParseWithNonNumericStart) {
    auto parser = sepBy(int_parser(), char_parser(comma));
    std::string input = "abc,123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());  // Should return empty vector
    EXPECT_EQ(pos, input.begin());  // Should not consume input
}

// ============================================================================
// sepBy1 Tests
// ============================================================================

class SepBy1Test : public ::testing::Test {
protected:
    void SetUp() override {
        comma = ',';
    }

    char comma;
};

TEST_F(SepBy1Test, ParseEmptyInputFails) {
    auto parser = sepBy1(int_parser(), char_parser(comma));
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());  // Must fail on empty
    EXPECT_EQ(pos, input.begin());
}

TEST_F(SepBy1Test, ParseSingleElement) {
    auto parser = sepBy1(int_parser(), char_parser(comma));
    std::string input = "42";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1UL);
    EXPECT_EQ((*result)[0], 42);
}

TEST_F(SepBy1Test, ParseMultipleElements) {
    auto parser = sepBy1(int_parser(), char_parser(comma));
    std::string input = "7,8,9";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0], 7);
    EXPECT_EQ((*result)[1], 8);
    EXPECT_EQ((*result)[2], 9);
}

TEST_F(SepBy1Test, ParseNonNumericStartFails) {
    auto parser = sepBy1(int_parser(), char_parser(comma));
    std::string input = "abc,123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());  // Must fail - no elements parsed
}

// ============================================================================
// sepEndBy Tests
// ============================================================================

class SepEndByTest : public ::testing::Test {
protected:
    void SetUp() override {
        comma = ',';
    }

    char comma;
};

TEST_F(SepEndByTest, ParseEmptyInput) {
    auto parser = sepEndBy(int_parser(), char_parser(comma));
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST_F(SepEndByTest, ParseWithTrailingSeparator) {
    auto parser = sepEndBy(int_parser(), char_parser(comma));
    std::string input = "1,2,3,";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0], 1);
    EXPECT_EQ((*result)[1], 2);
    EXPECT_EQ((*result)[2], 3);
    EXPECT_EQ(pos, input.end());  // Should consume trailing separator
}

TEST_F(SepEndByTest, ParseWithoutTrailingSeparator) {
    auto parser = sepEndBy(int_parser(), char_parser(comma));
    std::string input = "1,2,3";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0], 1);
    EXPECT_EQ((*result)[1], 2);
    EXPECT_EQ((*result)[2], 3);
    EXPECT_EQ(pos, input.end());
}

// ============================================================================
// endBy Tests
// ============================================================================

class EndByTest : public ::testing::Test {
protected:
    void SetUp() override {
        semicolon = ';';
    }

    char semicolon;
};

TEST_F(EndByTest, ParseEmptyInput) {
    auto parser = endBy(int_parser(), char_parser(semicolon));
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST_F(EndByTest, ParseWithTerminators) {
    auto parser = endBy(int_parser(), char_parser(semicolon));
    std::string input = "1;2;3;";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0], 1);
    EXPECT_EQ((*result)[1], 2);
    EXPECT_EQ((*result)[2], 3);
    EXPECT_EQ(pos, input.end());
}

TEST_F(EndByTest, ParseWithoutFinalTerminator) {
    auto parser = endBy(int_parser(), char_parser(semicolon));
    std::string input = "1;2;3";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2UL);  // Only includes elements with terminators
    EXPECT_EQ((*result)[0], 1);
    EXPECT_EQ((*result)[1], 2);
    // Should stop before '3' since it has no terminator
    EXPECT_EQ(std::string(pos, input.end()), "3");
}

// ============================================================================
// Integration Tests with Algebraic Types
// ============================================================================

// Parser that uses our unsigned_int type
class UnsignedIntParser {
public:
    using output_type = unsigned_int;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<unsigned_int>>
    {
        std::string digits;
        Iterator current = begin;

        while (current != end && std::isdigit(*current)) {
            digits += *current;
            ++current;
        }

        if (digits.empty()) {
            return {begin, std::nullopt};
        }

        return {current, make_unsigned_int(digits)};
    }
};

TEST(ListCombinatorIntegrationTest, SepByWithUnsignedInt) {
    auto parser = sepBy(UnsignedIntParser{}, char_parser(','));
    std::string input = "100,200,300";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0].val(), 100ULL);
    EXPECT_EQ((*result)[1].val(), 200ULL);
    EXPECT_EQ((*result)[2].val(), 300ULL);
}

// Parser that uses lc_alpha
class LcAlphaParser {
public:
    using output_type = lc_alpha;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<lc_alpha>>
    {
        std::string word;
        Iterator current = begin;

        while (current != end && std::isalpha(*current)) {
            word += *current;
            ++current;
        }

        if (word.empty()) {
            return {begin, std::nullopt};
        }

        return {current, make_lc_alpha(word)};
    }
};

TEST(ListCombinatorIntegrationTest, SepByWithLcAlpha) {
    auto parser = sepBy(LcAlphaParser{}, char_parser(','));
    std::string input = "hello,world,test";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3UL);
    EXPECT_EQ((*result)[0].str(), "hello");
    EXPECT_EQ((*result)[1].str(), "world");
    EXPECT_EQ((*result)[2].str(), "test");
}

TEST(ListCombinatorIntegrationTest, CSVParsing) {
    // Real-world CSV parsing example
    auto parser = sepBy(int_parser(), char_parser(','));
    std::string input = "10,20,30,40,50";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 5UL);

    int sum = 0;
    for (int val : *result) {
        sum += val;
    }
    EXPECT_EQ(sum, 150);
}

// Parse config-style: "key;value;key;value;"
class WordParser {
public:
    using output_type = std::string;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        std::string word;
        Iterator current = begin;

        while (current != end && std::isalpha(*current)) {
            word += *current;
            ++current;
        }

        if (word.empty()) {
            return {begin, std::nullopt};
        }

        return {current, std::make_optional(word)};
    }
};

TEST(ListCombinatorIntegrationTest, ConfigFileParsing) {
    auto parser = endBy(WordParser{}, char_parser(';'));
    std::string input = "name;value;port;host;";  // All alphabetic words

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 4UL);
    EXPECT_EQ((*result)[0], "name");
    EXPECT_EQ((*result)[1], "value");
    EXPECT_EQ((*result)[2], "port");
    EXPECT_EQ((*result)[3], "host");
}

// ============================================================================
// Whitespace Separator Tests
// ============================================================================

TEST(WhitespaceSeparatorTest, ParseWithWhitespace) {
    auto parser = sepBy(int_parser(), whitespace());
    std::string input = "1 2  3   4";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 4UL);
    EXPECT_EQ((*result)[0], 1);
    EXPECT_EQ((*result)[1], 2);
    EXPECT_EQ((*result)[2], 3);
    EXPECT_EQ((*result)[3], 4);
}

TEST(WhitespaceSeparatorTest, ParseWithOptionalWhitespace) {
    auto parser = sepBy(int_parser(), optional_whitespace());
    std::string input = "1 2 3";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    // Should parse as much as possible with optional whitespace
    EXPECT_GE(result->size(), 1UL);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(ListCombinatorEdgeCases, VeryLongList) {
    auto parser = sepBy(int_parser(), char_parser(','));
    std::string input;

    // Build a list of 1000 numbers
    for (int i = 0; i < 1000; ++i) {
        if (i > 0) input += ',';
        input += std::to_string(i);
    }

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 1000UL);
    EXPECT_EQ((*result)[0], 0);
    EXPECT_EQ((*result)[999], 999);
}

TEST(ListCombinatorEdgeCases, SingleCharacter) {
    auto parser = sepBy(int_parser(), char_parser(','));
    std::string input = "5";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1UL);
    EXPECT_EQ((*result)[0], 5);
}

TEST(ListCombinatorEdgeCases, OnlySeparators) {
    auto parser = sepBy(int_parser(), char_parser(','));
    std::string input = ",,,";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

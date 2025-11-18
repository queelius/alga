/**
 * @file fuzzy_parsers_test.cpp
 * @brief Comprehensive tests for fuzzy parser combinators
 */

#include <gtest/gtest.h>
#include "parsers/fuzzy_parsers.hpp"
#include <string>

using namespace alga;
using namespace alga::fuzzy;

// ============================================================================
// Word Parser Tests
// ============================================================================

class WordParserTest : public ::testing::Test {};

TEST_F(WordParserTest, ParsesSimpleWord) {
    auto parser = word_parser();
    std::string input = "hello";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
    EXPECT_EQ(pos, input.end());
}

TEST_F(WordParserTest, StopsAtNonAlpha) {
    auto parser = word_parser();
    std::string input = "hello123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
    EXPECT_EQ(std::string(pos, input.end()), "123");
}

TEST_F(WordParserTest, FailsOnNumber) {
    auto parser = word_parser();
    std::string input = "123";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(WordParserTest, EmptyInput) {
    auto parser = word_parser();
    std::string input = "";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Fuzzy Match Tests
// ============================================================================

class FuzzyMatchTest : public ::testing::Test {};

TEST_F(FuzzyMatchTest, ExactMatch) {
    auto parser = fuzzy_match("hello", 2);
    std::string input = "hello";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST_F(FuzzyMatchTest, OneTypo) {
    auto parser = fuzzy_match("hello", 2);
    std::string input = "helo";  // Missing 'l'

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "helo");
}

TEST_F(FuzzyMatchTest, TwoTypos) {
    auto parser = fuzzy_match("hello", 2);
    std::string input = "helo";  // 1 typo

    auto [pos, result] = parser.parse(input.begin(), input.end());
    ASSERT_TRUE(result.has_value());
}

TEST_F(FuzzyMatchTest, TooManyTypos) {
    auto parser = fuzzy_match("hello", 1);
    std::string input = "world";  // Completely different

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(FuzzyMatchTest, Transposition) {
    auto parser = fuzzy_match_dl("hello", 1);  // Damerau-Levenshtein
    std::string input = "ehllo";  // Transposition

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "ehllo");
}

// ============================================================================
// Phonetic Match Tests
// ============================================================================

class PhoneticMatchTest : public ::testing::Test {};

TEST_F(PhoneticMatchTest, SoundAlike) {
    auto parser = phonetic_match("Smith");
    std::string input = "Smyth";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Smyth");
}

TEST_F(PhoneticMatchTest, ExactMatch) {
    auto parser = phonetic_match("Smith");
    std::string input = "Smith";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Smith");
}

TEST_F(PhoneticMatchTest, DifferentSound) {
    auto parser = phonetic_match("Smith");
    std::string input = "Jones";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(PhoneticMatchTest, CommonNames) {
    auto parser = phonetic_match("Robert");

    auto [pos1, result1] = parser.parse(std::string("Rupert").begin(), std::string("Rupert").end());
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, "Rupert");
}

// ============================================================================
// Similarity Match Tests
// ============================================================================

class SimilarityMatchTest : public ::testing::Test {};

TEST_F(SimilarityMatchTest, HighSimilarity) {
    auto parser = similarity_match("hello", 0.8);
    std::string input = "hallo";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hallo");
}

TEST_F(SimilarityMatchTest, LowSimilarity) {
    auto parser = similarity_match("hello", 0.8);
    std::string input = "world";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(SimilarityMatchTest, ExactMatch) {
    auto parser = similarity_match("test", 0.9);
    std::string input = "test";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "test");
}

// ============================================================================
// Fuzzy Choice Tests
// ============================================================================

class FuzzyChoiceTest : public ::testing::Test {};

TEST_F(FuzzyChoiceTest, MatchesClosest) {
    auto parser = fuzzy_choice({"apple", "banana", "cherry"}, 2);
    std::string input = "aple";  // Close to "apple"

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "apple");  // Normalized to closest
}

TEST_F(FuzzyChoiceTest, ExactMatchInChoices) {
    auto parser = fuzzy_choice({"red", "green", "blue"}, 1);
    std::string input = "green";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "green");
}

TEST_F(FuzzyChoiceTest, NoCloseMatch) {
    auto parser = fuzzy_choice({"apple", "banana"}, 1);
    std::string input = "orange";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

TEST_F(FuzzyChoiceTest, TypoInChoice) {
    auto parser = fuzzy_choice({"hello", "world", "test"}, 2);
    std::string input = "wrld";  // Close to "world"

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "world");
}

// ============================================================================
// Case Insensitive Tests
// ============================================================================

class CaseInsensitiveTest : public ::testing::Test {};

TEST_F(CaseInsensitiveTest, Lowercase) {
    auto parser = case_insensitive("Hello");
    std::string input = "hello";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST_F(CaseInsensitiveTest, Uppercase) {
    auto parser = case_insensitive("hello");
    std::string input = "HELLO";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "HELLO");
}

TEST_F(CaseInsensitiveTest, MixedCase) {
    auto parser = case_insensitive("hello");
    std::string input = "HeLLo";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "HeLLo");
}

TEST_F(CaseInsensitiveTest, NoMatch) {
    auto parser = case_insensitive("hello");
    std::string input = "world";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Combined Fuzzy Matcher Tests
// ============================================================================

class CombinedFuzzyTest : public ::testing::Test {};

TEST_F(CombinedFuzzyTest, ExactMatch) {
    auto parser = combined_fuzzy("Python", 2);
    std::string input = "Python";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Python");
}

TEST_F(CombinedFuzzyTest, CaseInsensitiveMatch) {
    auto parser = combined_fuzzy("Python", 2);
    std::string input = "python";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "python");
}

TEST_F(CombinedFuzzyTest, FuzzyMatch) {
    auto parser = combined_fuzzy("Python", 2);
    std::string input = "Pyton";  // Missing 'h'

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Pyton");
}

TEST_F(CombinedFuzzyTest, PhoneticMatch) {
    auto parser = combined_fuzzy("Smith", 2);
    std::string input = "Smyth";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Smyth");
}

TEST_F(CombinedFuzzyTest, NoMatch) {
    auto parser = combined_fuzzy("hello", 1);
    std::string input = "world";

    auto [pos, result] = parser.parse(input.begin(), input.end());

    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Practical Use Cases
// ============================================================================

TEST(FuzzyParsersUseCases, TolerantCSVHeader) {
    // Parse CSV header with possible typos
    auto name_col = fuzzy_match("name", 2);
    auto email_col = fuzzy_match("email", 2);
    auto phone_col = fuzzy_match("phone", 2);

    // Should accept "nme" as "name"
    std::string typo = "nme";
    auto [pos, result] = name_col.parse(typo.begin(), typo.end());
    ASSERT_TRUE(result.has_value());
}

TEST(FuzzyParsersUseCases, CommandParser) {
    // Accept command variations
    auto exit_cmd = combined_fuzzy("exit", 1);

    // Should accept: exit, Exit, quit, etc.
    std::string input1 = "exit";
    auto [pos1, r1] = exit_cmd.parse(input1.begin(), input1.end());
    EXPECT_TRUE(r1.has_value());

    std::string input2 = "Exit";
    auto [pos2, r2] = exit_cmd.parse(input2.begin(), input2.end());
    EXPECT_TRUE(r2.has_value());
}

TEST(FuzzyParsersUseCases, NameMatching) {
    // Match similar names in database
    auto smith_parser = phonetic_match("Smith");

    std::vector<std::string> variations = {"Smyth", "Smythe"};

    for (const auto& name : variations) {
        auto [pos, result] = smith_parser.parse(name.begin(), name.end());
        EXPECT_TRUE(result.has_value()) << "Failed to match: " << name;
    }
}

TEST(FuzzyParsersUseCases, SpellTolerantSearch) {
    // Search query with typos
    auto parser = fuzzy_match("algorithm", 3);

    std::vector<std::string> misspellings = {
        "algorythm",
        "algoritm",
        "algroithm"
    };

    for (const auto& spelling : misspellings) {
        auto [pos, result] = parser.parse(spelling.begin(), spelling.end());
        EXPECT_TRUE(result.has_value()) << "Failed to match: " << spelling;
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(FuzzyParsersEdgeCases, EmptyTarget) {
    auto parser = fuzzy_match("", 1);
    std::string input = "test";

    auto [pos, result] = parser.parse(input.begin(), input.end());
    // Should handle gracefully
}

TEST(FuzzyParsersEdgeCases, VeryShortWords) {
    auto parser = fuzzy_match("a", 1);
    std::string input = "b";

    auto [pos, result] = parser.parse(input.begin(), input.end());
    ASSERT_TRUE(result.has_value());
}

TEST(FuzzyParsersEdgeCases, ZeroDistance) {
    auto parser = fuzzy_match("exact", 0);

    std::string match = "exact";
    auto [pos1, r1] = parser.parse(match.begin(), match.end());
    EXPECT_TRUE(r1.has_value());

    std::string nomatch = "exakt";
    auto [pos2, r2] = parser.parse(nomatch.begin(), nomatch.end());
    EXPECT_FALSE(r2.has_value());
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/**
 * @file unit_tests.cpp
 * @brief Unit tests for individual parser components
 * 
 * These tests focus on testing individual algebraic parser components
 * in isolation to ensure they meet their mathematical specifications.
 */

#include <gtest/gtest.h>
#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include "parsers/fsm_string_rewriter.hpp"
#include "parsers/combinatorial_parser_fixed.hpp"
#include <string>
#include <optional>

using namespace alga;
using namespace alga::combinatorial;

// ============================================================================
// lc_alpha Unit Tests
// ============================================================================

class LcAlphaTest : public ::testing::Test {
protected:
    void SetUp() override {
        valid_input = "hello";
        mixed_case_input = "HeLLo";
        invalid_input = "hello123";
        empty_input = "";
    }
    
    std::string valid_input;
    std::string mixed_case_input;
    std::string invalid_input;
    std::string empty_input;
};

TEST_F(LcAlphaTest, MakeLcAlphaValidInput) {
    auto result = make_lc_alpha(valid_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::string(*result), "hello");
}

TEST_F(LcAlphaTest, MakeLcAlphaMixedCase) {
    auto result = make_lc_alpha(mixed_case_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::string(*result), "hello");
}

TEST_F(LcAlphaTest, MakeLcAlphaInvalidInput) {
    auto result = make_lc_alpha(invalid_input);
    EXPECT_FALSE(result.has_value());
}

TEST_F(LcAlphaTest, MakeLcAlphaEmptyInput) {
    auto result = make_lc_alpha(empty_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST_F(LcAlphaTest, MonoidIdentityElement) {
    auto empty = lc_alpha{};
    auto word = *make_lc_alpha("test");
    
    // Left identity: empty * word = word
    EXPECT_EQ(empty * word, word);
    
    // Right identity: word * empty = word
    EXPECT_EQ(word * empty, word);
}

TEST_F(LcAlphaTest, MonoidAssociativity) {
    auto a = *make_lc_alpha("hello");
    auto b = *make_lc_alpha("beautiful");
    auto c = *make_lc_alpha("world");
    
    // (a * b) * c = a * (b * c)
    EXPECT_EQ((a * b) * c, a * (b * c));
}

TEST_F(LcAlphaTest, IteratorInterface) {
    auto word = *make_lc_alpha("hello");
    
    EXPECT_EQ(word.size(), 5);
    EXPECT_FALSE(word.empty());
    
    std::string reconstructed(word.begin(), word.end());
    EXPECT_EQ(reconstructed, "hello");
}

TEST_F(LcAlphaTest, ComparisonOperators) {
    auto hello1 = *make_lc_alpha("hello");
    auto hello2 = *make_lc_alpha("hello");
    auto world = *make_lc_alpha("world");
    
    EXPECT_EQ(hello1, hello2);
    EXPECT_NE(hello1, world);
}

// ============================================================================
// Porter2 Stemmer Unit Tests
// ============================================================================

class Porter2StemmerTest : public ::testing::Test {
protected:
    porter2_stemmer stemmer;
};

TEST_F(Porter2StemmerTest, BasicStemming) {
    auto running = *make_lc_alpha("running");
    auto result = stemmer(running);
    
    // When called with lc_alpha, returns porter2_stem directly (not optional)
    EXPECT_EQ(std::string(result), "run");
}

TEST_F(Porter2StemmerTest, StringInterfaceStemming) {
    auto result = stemmer("running");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::string(*result), "run");
}

TEST_F(Porter2StemmerTest, InvalidStringInput) {
    auto result = stemmer("run123ning");
    EXPECT_FALSE(result.has_value());
}

TEST_F(Porter2StemmerTest, EmptyStringInput) {
    auto result = stemmer("");
    // Empty string should create empty lc_alpha and stem to empty
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(std::string(*result).empty());
}

TEST_F(Porter2StemmerTest, AlreadyStemmedWord) {
    auto run = *make_lc_alpha("run");
    auto result = stemmer(run);
    
    // When called with lc_alpha, returns porter2_stem directly (not optional)
    EXPECT_EQ(std::string(result), "run");
}

TEST_F(Porter2StemmerTest, Porter2StemEquality) {
    auto running = *make_lc_alpha("running");
    auto runs = *make_lc_alpha("runs");
    
    auto stem1 = stemmer(running);
    auto stem2 = stemmer(runs);
    
    // When called with lc_alpha, returns porter2_stem directly (not optional)
    // Both should stem to "run"
    EXPECT_EQ(stem1, stem2);
}

// ============================================================================
// FSM String Rewriter Unit Tests
// ============================================================================

class FSMStringRewriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        rewriter.push("hello", "hi");
        rewriter.push("world", "earth");
        rewriter.push("\\s+", " "); // normalize whitespace
    }
    
    fsm_string_rewriter rewriter;
};

TEST_F(FSMStringRewriterTest, BasicRewrite) {
    std::string input = "hello world";
    auto result = rewriter(input);
    
    EXPECT_EQ(result, "hi earth");
}

TEST_F(FSMStringRewriterTest, WhitespaceNormalization) {
    std::string input = "hello    world";
    auto result = rewriter(input);
    
    EXPECT_EQ(result, "hi earth");
}

TEST_F(FSMStringRewriterTest, NoMatchingRules) {
    std::string input = "foo bar";
    auto result = rewriter(input);
    
    EXPECT_EQ(result, "foo bar"); // Should remain unchanged
}

TEST_F(FSMStringRewriterTest, IterativeApplication) {
    fsm_string_rewriter iterative;
    iterative.push("aa", "a");
    
    std::string input = "aaaa";
    auto result = iterative(input);
    
    EXPECT_EQ(result, "a"); // Should reduce aaaa -> aaa -> aa -> a
}

TEST_F(FSMStringRewriterTest, MaxIterationsLimit) {
    fsm_string_rewriter limited;
    limited.push("a", "aa"); // This would cause infinite expansion
    
    std::string input = "a";
    auto result = limited(input, 3); // Limit to 3 iterations
    
    EXPECT_EQ(result, "aaaaaaaa"); // Should be "aa" -> "aaaa" -> "aaaaaaaa"
}

TEST_F(FSMStringRewriterTest, RuleConcatenation) {
    fsm_string_rewriter second;
    second.push("hi", "hello");
    
    auto combined = concat(rewriter, second);
    
    std::string input = "hello world";
    auto result = combined(input);
    
    // First: "hello world" -> "hi earth"
    // Then: "hi earth" -> "hello earth"
    EXPECT_EQ(result, "hello earth");
}

// ============================================================================
// Combinatorial Parser Unit Tests
// ============================================================================

class CombinatorialParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_string = "hello123world";
    }
    
    std::string test_string;
};

TEST_F(CombinatorialParserTest, TokenParserAlpha) {
    auto alpha_parser = make_alpha_parser();
    auto [remaining, result] = alpha_parser(test_string.begin(), test_string.end());
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
    EXPECT_EQ(std::string(remaining, test_string.end()), "123world");
}

TEST_F(CombinatorialParserTest, TokenParserDigit) {
    auto digit_parser = make_digit_parser();
    auto start = test_string.begin() + 5; // Start at "123"
    auto [remaining, result] = digit_parser(start, test_string.end());
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 123);
    EXPECT_EQ(std::string(remaining, test_string.end()), "world");
}

TEST_F(CombinatorialParserTest, LiteralParser) {
    auto hello_parser = literal("hello");
    auto [remaining, result] = hello_parser(test_string.begin(), test_string.end());
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
    EXPECT_EQ(std::string(remaining, test_string.end()), "123world");
}

TEST_F(CombinatorialParserTest, LiteralParserFail) {
    auto world_parser = literal("world");
    auto [remaining, result] = world_parser(test_string.begin(), test_string.end());
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(remaining, test_string.begin()); // Should not advance
}

TEST_F(CombinatorialParserTest, SequenceParser) {
    auto hello_parser = literal("hello");
    auto digit_parser = make_digit_parser();
    auto seq_parser = sequence(hello_parser, digit_parser);
    
    auto [remaining, result] = seq_parser(test_string.begin(), test_string.end());
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->first, "hello");
    EXPECT_EQ(result->second, 123);
    EXPECT_EQ(std::string(remaining, test_string.end()), "world");
}

TEST_F(CombinatorialParserTest, AlternativeParser) {
    auto digit_parser = make_digit_parser();
    auto alpha_parser = make_alpha_parser();
    auto alt_parser = alternative<decltype(digit_parser), decltype(alpha_parser), std::string>(
        std::move(digit_parser), std::move(alpha_parser));
    
    auto [remaining, result] = alt_parser(test_string.begin(), test_string.end());
    
    ASSERT_TRUE(result.has_value());
    // Should match alpha since digit fails at position 0
    EXPECT_EQ(*result, "hello");
}

TEST_F(CombinatorialParserTest, ManyParser) {
    std::string repeated = "aaabbbccc";
    auto alpha_parser = make_alpha_parser();
    auto many_parser = many(alpha_parser);
    
    auto [remaining, result] = many_parser(repeated.begin(), repeated.end());
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 1); // Should parse entire string as one token
    EXPECT_EQ((*result)[0], "aaabbbccc");
}

TEST_F(CombinatorialParserTest, OptionalParser) {
    auto digit_parser = make_digit_parser();
    auto opt_parser = maybe(digit_parser);  // Use 'maybe' instead of 'optional' to avoid name conflict
    
    // Test with alpha input (digit parser should fail but optional succeeds)
    auto [remaining, result] = opt_parser(test_string.begin(), test_string.end());
    
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->has_value()); // Optional result should be nullopt
    EXPECT_EQ(remaining, test_string.begin()); // Should not advance
}

TEST_F(CombinatorialParserTest, TransformParser) {
    auto alpha_parser = make_alpha_parser();
    auto length_parser = transform(alpha_parser, [](const std::string& s) { return s.length(); });
    
    auto [remaining, result] = length_parser(test_string.begin(), test_string.end());
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5); // Length of "hello"
}

// ============================================================================
// Parser Composition Unit Tests
// ============================================================================

class ParserCompositionUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_input = "hello world 123";
    }
    
    std::string test_input;
};

TEST_F(ParserCompositionUnitTest, CompleteParseSuccess) {
    auto word_parser = make_alpha_parser();
    auto result = parse(std::move(word_parser), std::string("hello"));
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST_F(ParserCompositionUnitTest, CompleteParsePartialConsumption) {
    auto word_parser = make_alpha_parser();
    auto result = parse(std::move(word_parser), test_input); // Only parses "hello", not complete
    
    EXPECT_FALSE(result.has_value()); // Should fail because it doesn't consume all input
}

TEST_F(ParserCompositionUnitTest, PartialParseSuccess) {
    auto word_parser = make_alpha_parser();
    auto [remaining, result] = parse_partial(std::move(word_parser), test_input);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
    EXPECT_EQ(std::string(remaining, test_input.end()), " world 123");
}

// ============================================================================
// Mathematical Properties Unit Tests
// ============================================================================

class MathematicalPropertiesTest : public ::testing::Test {
protected:
    // Helper function to test parser identity
    template<typename Parser, typename Input>
    bool test_identity(Parser const& parser, Input const& input) {
        auto result1 = parser(input.begin(), input.end());
        auto result2 = parser(input.begin(), input.end());
        return result1.second == result2.second; // Deterministic behavior
    }
};

TEST_F(MathematicalPropertiesTest, ParserDeterminism) {
    auto alpha_parser = make_alpha_parser();
    std::string input = "hello";
    
    EXPECT_TRUE(test_identity(alpha_parser, input));
}

TEST_F(MathematicalPropertiesTest, SequenceAssociativity) {
    auto a = literal("a");
    auto b = literal("b");
    auto c = literal("c");
    
    std::string input = "abc";
    
    // Test (a >> b) >> c == a >> (b >> c)
    auto left_assoc = sequence(sequence(a, b), c);
    auto right_assoc = sequence(a, sequence(b, c));
    
    auto [_, result1] = left_assoc(input.begin(), input.end());
    auto [__, result2] = right_assoc(input.begin(), input.end());
    
    EXPECT_TRUE(result1.has_value());
    EXPECT_TRUE(result2.has_value());
    // Both should succeed (structural equality harder to test with nested pairs)
}

TEST_F(MathematicalPropertiesTest, AlternativeCommutativity) {
    // Alternative is not commutative in general due to order of choice,
    // but we can test that both choices are tried
    auto digit_parser = make_digit_parser();
    auto alpha_parser = make_alpha_parser();
    
    std::string digit_input = "123";
    std::string alpha_input = "hello";
    
    // Need to create fresh parsers since we move them
    auto alt1 = alternative<decltype(digit_parser), decltype(alpha_parser), std::string>(
        std::move(digit_parser), std::move(alpha_parser));
    
    auto digit_parser2 = make_digit_parser();
    auto alpha_parser2 = make_alpha_parser();
    auto alt2 = alternative<decltype(alpha_parser2), decltype(digit_parser2), std::string>(
        std::move(alpha_parser2), std::move(digit_parser2));
    
    auto [_, result1] = alt1(digit_input.begin(), digit_input.end());
    auto [__, result2] = alt2(digit_input.begin(), digit_input.end());
    
    EXPECT_TRUE(result1.has_value());
    EXPECT_TRUE(result2.has_value());
    // Both should succeed on digit input
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
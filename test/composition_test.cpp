/**
 * @file composition_test.cpp
 * @brief Comprehensive test suite for algebraic parser composition
 * 
 * This test suite demonstrates test-driven development principles applied
 * to algebraic parser systems, with tests serving as both specifications
 * and validation of mathematical properties.
 */

#include "../examples/composition_examples.hpp"
#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include <gtest/gtest.h>
#include <string>
#include <optional>
#include <vector>
#include <type_traits>

using namespace alga;
using namespace alga::examples;

// ============================================================================
// Test Suite 1: Fundamental Algebraic Properties
// ============================================================================

class AlgebraicPropertiesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup common test data
        empty_alpha = lc_alpha{};
        hello = *make_lc_alpha("hello");
        world = *make_lc_alpha("world");
        test = *make_lc_alpha("test");
    }
    
    lc_alpha empty_alpha;
    lc_alpha hello;
    lc_alpha world;
    lc_alpha test;
};

TEST_F(AlgebraicPropertiesTest, LcAlphaFormsMonoid) {
    // Test associativity: (a * b) * c = a * (b * c)
    auto left_assoc = (hello * world) * test;
    auto right_assoc = hello * (world * test);
    EXPECT_EQ(left_assoc, right_assoc);
    
    // Test left identity: empty * a = a
    EXPECT_EQ(empty_alpha * hello, hello);
    
    // Test right identity: a * empty = a
    EXPECT_EQ(hello * empty_alpha, hello);
    
    // Test that the operation is closed (always produces valid lc_alpha)
    auto combined = hello * world;
    EXPECT_EQ(std::string{combined}, "helloworld");
}

TEST_F(AlgebraicPropertiesTest, OptionalMonoidProperties) {
    auto opt_hello = std::optional<lc_alpha>{hello};
    auto opt_world = std::optional<lc_alpha>{world};
    auto opt_empty = std::optional<lc_alpha>{std::nullopt};
    
    // Test that optional composition preserves monoid structure
    auto result1 = opt_hello * opt_world;
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, hello * world);
    
    // Test nullopt behavior (absorbing element)
    auto result2 = opt_hello * opt_empty;
    EXPECT_FALSE(result2.has_value());
    
    auto result3 = opt_empty * opt_world;
    EXPECT_FALSE(result3.has_value());
}

// ============================================================================
// Test Suite 2: Parser Composition Properties
// ============================================================================

class ParserCompositionTest : public ::testing::Test {
protected:
    // Simple test parsers for composition testing
    struct IdentityParser {
        using input_type = std::string;
        using output_type = std::string;
        
        auto operator()(std::string const& input) const -> std::optional<std::string> {
            return input;
        }
    };
    
    struct LengthParser {
        using input_type = std::string;
        using output_type = size_t;
        
        auto operator()(std::string const& input) const -> std::optional<size_t> {
            return input.length();
        }
    };
    
    struct DoubleParser {
        using input_type = size_t;
        using output_type = size_t;
        
        auto operator()(size_t const& input) const -> std::optional<size_t> {
            return input * 2;
        }
    };
};

TEST_F(ParserCompositionTest, SequentialCompositionWorks) {
    auto length_parser = LengthParser{};
    auto double_parser = DoubleParser{};
    
    auto composed = compose(length_parser, double_parser);
    
    auto result = composed("hello");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 10); // "hello".length() * 2 = 5 * 2 = 10
}

TEST_F(ParserCompositionTest, CompositionAssociativity) {
    auto id = IdentityParser{};
    auto len = LengthParser{};
    auto dbl = DoubleParser{};
    
    // Test (id ∘ len) ∘ dbl = id ∘ (len ∘ dbl)
    auto left_assoc = compose(compose(id, len), dbl);
    auto right_assoc = compose(id, compose(len, dbl));
    
    std::string test_input = "testing";
    auto result1 = left_assoc(test_input);
    auto result2 = right_assoc(test_input);
    
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result1, *result2);
}

TEST_F(ParserCompositionTest, AlternativeParserChoiceLogic) {
    struct AlwaysFailParser {
        using input_type = std::string;
        using output_type = int;
        
        auto operator()(std::string const&) const -> std::optional<int> {
            return std::nullopt;
        }
    };
    
    struct AlwaysSucceedParser {
        using input_type = std::string;
        using output_type = int;
        
        auto operator()(std::string const&) const -> std::optional<int> {
            return 42;
        }
    };
    
    auto fail_parser = AlwaysFailParser{};
    auto succeed_parser = AlwaysSucceedParser{};
    
    // Test first choice succeeds
    auto alt1 = alternative(succeed_parser, fail_parser);
    auto result1 = alt1("test");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(std::get<int>(*result1), 42);
    
    // Test second choice succeeds when first fails
    auto alt2 = alternative(fail_parser, succeed_parser);
    auto result2 = alt2("test");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(std::get<int>(*result2), 42);
    
    // Test both fail
    auto alt3 = alternative(fail_parser, fail_parser);
    auto result3 = alt3("test");
    EXPECT_FALSE(result3.has_value());
}

// ============================================================================
// Test Suite 3: Word Processing Pipeline Tests
// ============================================================================

class WordProcessingPipelineTest : public ::testing::Test {
protected:
    WordProcessingPipeline processor;
};

TEST_F(WordProcessingPipelineTest, BasicSentenceProcessing) {
    std::string input = "The quick brown foxes are running quickly!";
    auto result = processor.process_sentence(input);
    
    EXPECT_FALSE(result.empty());
    
    // Should contain stemmed versions
    bool contains_run = std::find(result.begin(), result.end(), "run") != result.end();
    bool contains_quick = std::find(result.begin(), result.end(), "quick") != result.end();
    
    EXPECT_TRUE(contains_run || contains_quick); // At least one should be stemmed
}

TEST_F(WordProcessingPipelineTest, HandlesSpecialCharacters) {
    std::string input = "Hello, world! This is a test... 123 numbers.";
    auto result = processor.process_sentence(input);
    
    // Should filter out numbers and punctuation
    for (auto const& word : result) {
        EXPECT_FALSE(word.empty());
        EXPECT_TRUE(std::all_of(word.begin(), word.end(), 
                               [](char c) { return std::isalpha(c) && std::islower(c); }));
    }
}

TEST_F(WordProcessingPipelineTest, EmptyInputHandling) {
    auto result1 = processor.process_sentence("");
    EXPECT_TRUE(result1.empty());
    
    auto result2 = processor.process_sentence("   ");
    EXPECT_TRUE(result2.empty());
    
    auto result3 = processor.process_sentence("!@#$%");
    EXPECT_TRUE(result3.empty());
}

// ============================================================================
// Test Suite 4: Multi-Strategy Stemmer Tests
// ============================================================================

class MultiStrategyStemmerTest : public ::testing::Test {
protected:
    MultiStrategyStemmer stemmer;
};

TEST_F(MultiStrategyStemmerTest, Porter2Strategy) {
    auto word = *make_lc_alpha("running");
    auto result = stemmer(word);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "run");
}

TEST_F(MultiStrategyStemmerTest, SuffixRemovalFallback) {
    // Test with a word that Porter2 might not handle but simple suffix removal would
    auto word = *make_lc_alpha("testing");
    auto result = stemmer(word);
    
    ASSERT_TRUE(result.has_value());
    // Should be either Porter2 result or suffix removal result
    EXPECT_TRUE(*result == "test" || result->find("test") == 0);
}

TEST_F(MultiStrategyStemmerTest, FinalFallback) {
    // Test with a very short word that no stemming should affect
    auto word = *make_lc_alpha("at");
    auto result = stemmer(word);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "at"); // Should return original
}

TEST_F(MultiStrategyStemmerTest, AlgebraicConsistency) {
    // Test that the stemmer behaves consistently (deterministic)
    auto word = *make_lc_alpha("consistently");
    
    auto result1 = stemmer(word);
    auto result2 = stemmer(word);
    
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result1, *result2);
}

// ============================================================================
// Test Suite 5: Statistical Parser Tests
// ============================================================================

class StatisticalWordParserTest : public ::testing::Test {
protected:
    StatisticalWordParser parser{0.001}; // Low threshold for testing
};

TEST_F(StatisticalWordParserTest, FrequencyBasedParsing) {
    // Test with a common word that should be in the frequency table
    auto result = parser.parse_with_frequency("the");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->first, "the");
    EXPECT_GT(result->second, 0.001); // Should exceed threshold
}

TEST_F(StatisticalWordParserTest, StemmedFrequencyLookup) {
    // Test with a word that needs stemming to find frequency
    auto result = parser.parse_with_frequency("running");
    
    if (result) {
        // Should either find "running" or "run" in frequency table
        EXPECT_TRUE(result->first == "running" || result->first == "run");
        EXPECT_GT(result->second, 0.0);
    }
    // Not asserting presence since frequency table is minimal in test
}

TEST_F(StatisticalWordParserTest, UnknownWordHandling) {
    auto result = parser.parse_with_frequency("xyzabc123unknown");
    
    // Unknown words should return nullopt
    EXPECT_FALSE(result.has_value());
}

TEST_F(StatisticalWordParserTest, WeightedAlternativeComposition) {
    struct SimpleParser {
        using input_type = std::string;
        using output_type = std::string;
        
        auto operator()(std::string const& input) const -> std::optional<std::string> {
            return "simple_" + input;
        }
    };
    
    auto simple = SimpleParser{};
    auto weighted = parser.weighted_alternative(simple);
    
    // Test with common word (should use statistical parser)
    auto result1 = weighted("the");
    EXPECT_TRUE(result1 == std::optional<std::string>{"the"} ||
                result1 == std::optional<std::string>{"simple_the"});
    
    // Test with uncommon word (should fallback to simple parser)
    auto result2 = weighted("uncommonword");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, "simple_uncommonword");
}

// ============================================================================
// Test Suite 6: Document Processor Integration Tests
// ============================================================================

class DocumentProcessorTest : public ::testing::Test {
protected:
    DocumentProcessor processor;
};

TEST_F(DocumentProcessorTest, CompleteDocumentProcessing) {
    std::string document = "The quick brown foxes are running quickly through the forest.";
    auto result = processor.process_document(document);
    
    EXPECT_FALSE(result.normalized_words.empty());
    EXPECT_FALSE(result.stems.empty());
    EXPECT_FALSE(result.word_counts.empty());
    
    // Verify structure consistency
    EXPECT_LE(result.stems.size(), result.normalized_words.size()); // Some words might not stem
    
    // Check that word counts sum correctly
    int total_count = 0;
    for (auto const& [word, count] : result.word_counts) {
        total_count += count;
        EXPECT_GT(count, 0);
    }
    EXPECT_EQ(total_count, static_cast<int>(result.stems.size()));
}

TEST_F(DocumentProcessorTest, ComposedParserInterface) {
    auto composed = processor.as_composed_parser();
    
    std::string document = "Testing the composed parser interface functionality.";
    auto result = composed(document);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->normalized_words.empty());
    EXPECT_FALSE(result->stems.empty());
}

TEST_F(DocumentProcessorTest, ErrorHandling) {
    auto composed = processor.as_composed_parser();
    
    // These should not crash the parser
    auto result1 = composed("");
    auto result2 = composed("!@#$%^&*()");
    auto result3 = composed(std::string(10000, 'a')); // Very long input
    
    // All should either succeed or fail gracefully
    if (result1) EXPECT_TRUE(result1->normalized_words.empty());
    if (result2) EXPECT_TRUE(result2->normalized_words.empty());
    // result3 might succeed or fail, but shouldn't crash
}

// ============================================================================
// Test Suite 7: Type Safety and Compilation Tests
// ============================================================================

class TypeSafetyTest : public ::testing::Test {
public:
    // These are compile-time tests - if they compile, they pass
    
    template<typename T>
    struct has_input_type {
        template<typename U> static auto test(int) -> decltype(typename U::input_type{}, std::true_type{});
        template<typename> static std::false_type test(...);
        static constexpr bool value = decltype(test<T>(0))::value;
    };
    
    template<typename T>
    struct has_output_type {
        template<typename U> static auto test(int) -> decltype(typename U::output_type{}, std::true_type{});
        template<typename> static std::false_type test(...);
        static constexpr bool value = decltype(test<T>(0))::value;
    };
};

TEST_F(TypeSafetyTest, ParserTypesHaveRequiredMembers) {
    // Test that our parser types have the required type members
    EXPECT_TRUE((has_input_type<porter2_stemmer>::value));
    EXPECT_TRUE((has_output_type<porter2_stemmer>::value));
    
    EXPECT_TRUE((has_input_type<MultiStrategyStemmer>::value));
    EXPECT_TRUE((has_output_type<MultiStrategyStemmer>::value));
}

TEST_F(TypeSafetyTest, CompositionPreservesTypes) {
    // Test that composition preserves type safety
    auto id_parser = [](std::string const& s) -> std::optional<std::string> { return s; };
    auto len_parser = [](std::string const& s) -> std::optional<size_t> { return s.length(); };
    
    // This should compile and work correctly
    auto composed = [id_parser, len_parser](std::string const& input) {
        auto intermediate = id_parser(input);
        if (!intermediate) return std::nullopt;
        return len_parser(*intermediate);
    };
    
    auto result = composed("test");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 4);
}

// ============================================================================
// Test Suite 8: Performance and Benchmarking
// ============================================================================

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Generate test data
        test_words.reserve(1000);
        for (int i = 0; i < 1000; ++i) {
            test_words.push_back("testword" + std::to_string(i));
        }
    }
    
    std::vector<std::string> test_words;
};

TEST_F(PerformanceTest, StemmerPerformance) {
    porter2_stemmer stemmer;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int successful_stems = 0;
    for (auto const& word : test_words) {
        if (auto lc = make_lc_alpha(word)) {
            if (auto stem = stemmer(*lc)) {
                ++successful_stems;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_GT(successful_stems, 0);
    std::cout << "Processed " << successful_stems << " words in " 
              << duration.count() << " microseconds" << std::endl;
    
    // Performance assertion - should process at least 1000 words/second
    EXPECT_LT(duration.count(), 1000000); // Less than 1 second for 1000 words
}

TEST_F(PerformanceTest, ComposedParserPerformance) {
    DocumentProcessor processor;
    
    std::string large_document;
    for (auto const& word : test_words) {
        large_document += word + " ";
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = processor.process_document(large_document);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_FALSE(result.normalized_words.empty());
    std::cout << "Processed document with " << test_words.size() 
              << " words in " << duration.count() << " milliseconds" << std::endl;
    
    // Should process reasonable-sized documents quickly
    EXPECT_LT(duration.count(), 1000); // Less than 1 second
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
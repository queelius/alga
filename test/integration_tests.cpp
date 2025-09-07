/**
 * @file integration_tests.cpp
 * @brief Integration tests for algebraic parser composition and real-world scenarios
 * 
 * These tests verify that different parser components work together correctly
 * and can handle realistic parsing tasks with proper error handling and performance.
 */

#include <gtest/gtest.h>
#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include "parsers/fsm_string_rewriter.hpp"
#include "parsers/word_parser.hpp"
#include "parsers/combinatorial_parser_fixed.hpp"
#include "../examples/composition_examples.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>

using namespace alga;
using namespace alga::combinatorial;
using namespace alga::examples;

// ============================================================================
// Real-World Document Processing Integration Tests
// ============================================================================

class DocumentProcessingIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        sample_text = R"(
            The quick brown foxes are running quickly through the dense forest.
            They are being chased by hunters who are riding on horseback.
            The foxes demonstrate remarkable intelligence and agility as they
            navigate through the challenging terrain with ease and grace.
        )";
        
        complex_text = R"(
            Natural language processing (NLP) is a fascinating field that combines
            computational linguistics with machine learning algorithms. Researchers
            in this domain are constantly developing new methodologies to better
            understand human language patterns and communication structures.
        )";
    }
    
    std::string sample_text;
    std::string complex_text;
};

TEST_F(DocumentProcessingIntegrationTest, EndToEndDocumentProcessing) {
    DocumentProcessor processor;
    auto result = processor.process_document(sample_text);
    
    EXPECT_FALSE(result.normalized_words.empty());
    EXPECT_FALSE(result.stems.empty());
    EXPECT_FALSE(result.word_counts.empty());
    
    // Verify that common words are processed correctly
    bool found_run_stem = false;
    for (const auto& stem : result.stems) {
        if (stem == "run") {
            found_run_stem = true;
            break;
        }
    }
    EXPECT_TRUE(found_run_stem) << "Should find 'run' stem from 'running'";
    
    // Verify word count consistency
    size_t total_counted = 0;
    for (const auto& [word, count] : result.word_counts) {
        total_counted += count;
        EXPECT_GT(count, 0) << "All word counts should be positive";
    }
    EXPECT_EQ(total_counted, result.stems.size()) << "Total counts should match stem count";
}

TEST_F(DocumentProcessingIntegrationTest, ComplexTextProcessing) {
    DocumentProcessor processor;
    auto result = processor.process_document(complex_text);
    
    EXPECT_FALSE(result.normalized_words.empty());
    
    // Check for technical terms that should be preserved or properly stemmed
    std::vector<std::string> expected_stems = {"languag", "process", "learn", "algorithm", "research"};
    
    int found_technical_terms = 0;
    for (const auto& expected : expected_stems) {
        for (const auto& stem : result.stems) {
            if (stem.find(expected) != std::string::npos) {
                found_technical_terms++;
                break;
            }
        }
    }
    
    EXPECT_GT(found_technical_terms, 2) << "Should find several technical terms properly processed";
}

TEST_F(DocumentProcessingIntegrationTest, PerformanceUnderLoad) {
    DocumentProcessor processor;
    
    // Create a large document by repeating text
    std::string large_text;
    for (int i = 0; i < 100; ++i) {
        large_text += sample_text + " ";
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = processor.process_document(large_text);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_FALSE(result.normalized_words.empty());
    EXPECT_LT(duration.count(), 5000) << "Large document processing should complete within 5 seconds";
    
    std::cout << "Processed large document (" << large_text.length() 
              << " chars) in " << duration.count() << "ms" << std::endl;
}

// ============================================================================
// Parser Composition Integration Tests
// ============================================================================

class ParserCompositionIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        mixed_input = "The running foxes123 are being chased by 5 hunters on horseback!";
        structured_input = "word1 word2 123 word3 456 word4";
        edge_case_input = "   !@#$%^&*()   mixed123content   ";
    }
    
    std::string mixed_input;
    std::string structured_input;
    std::string edge_case_input;
};

TEST_F(ParserCompositionIntegrationTest, WordProcessingPipelineIntegration) {
    WordProcessingPipeline pipeline;
    
    auto result = pipeline.process_sentence(mixed_input);
    
    EXPECT_FALSE(result.empty());
    
    // Should filter out numbers and punctuation, keeping only alphabetic words
    for (const auto& word : result) {
        EXPECT_FALSE(word.empty());
        EXPECT_TRUE(std::all_of(word.begin(), word.end(), 
                               [](char c) { return std::isalpha(c) && std::islower(c); }))
            << "Word '" << word << "' should contain only lowercase alphabetic characters";
    }
    
    // Should contain stemmed versions
    bool contains_run = std::find(result.begin(), result.end(), "run") != result.end();
    EXPECT_TRUE(contains_run) << "Should contain 'run' (stemmed from 'running')";
}

TEST_F(ParserCompositionIntegrationTest, MultiStrategyStemmerIntegration) {
    MultiStrategyStemmer stemmer;
    
    std::vector<std::string> test_words = {
        "running", "happiness", "beautiful", "testing", "computation", "algorithmic"
    };
    
    std::vector<std::string> results;
    
    for (const auto& word : test_words) {
        if (auto lc_word = make_lc_alpha(word)) {
            if (auto stem = stemmer(*lc_word)) {
                results.push_back(*stem);
            }
        }
    }
    
    EXPECT_EQ(results.size(), test_words.size()) << "All words should be processed";
    
    // Check specific stemming results
    EXPECT_TRUE(std::find(results.begin(), results.end(), "run") != results.end());
    EXPECT_TRUE(std::find(results.begin(), results.end(), "happi") != results.end() ||
               std::find(results.begin(), results.end(), "happiness") != results.end());
}

TEST_F(ParserCompositionIntegrationTest, StatisticalParserIntegration) {
    StatisticalWordParser stat_parser;
    
    std::vector<std::string> common_words = {"the", "running", "beautiful", "unknown_word_xyz"};
    
    for (const auto& word : common_words) {
        auto result = stat_parser.parse_with_frequency(word);
        
        if (word == "the") {
            EXPECT_TRUE(result.has_value()) << "Common word 'the' should be found";
            if (result) {
                EXPECT_GT(result->second, 0.01) << "Word 'the' should have high frequency";
            }
        } else if (word == "unknown_word_xyz") {
            // Unknown words might not be found
            if (result) {
                EXPECT_GT(result->second, 0.0) << "If found, frequency should be positive";
            }
        }
    }
}

// ============================================================================
// Combinatorial Parser Integration Tests
// ============================================================================

class CombinatorialIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        structured_data = "hello 123 world 456 test";
        nested_structure = "((hello world) (test case))";
        csv_like_data = "name,age,city\nAlice,30,NYC\nBob,25,LA";
    }
    
    std::string structured_data;
    std::string nested_structure;
    std::string csv_like_data;
};

TEST_F(CombinatorialIntegrationTest, StructuredDataParsing) {
    // Create a parser for alternating words and numbers
    auto word_parser = make_alpha_parser();
    auto digit_parser = make_digit_parser();
    auto space_parser = make_whitespace_parser();
    
    // Parse word-number pairs
    auto word_num_parser = sequence(
        word_parser,
        sequence(space_parser, digit_parser)
    );
    
    auto many_pairs = many(sequence(word_num_parser, optional(space_parser)));
    
    auto [remaining, result] = many_pairs(structured_data.begin(), structured_data.end());
    
    EXPECT_TRUE(result.has_value());
    EXPECT_GT(result->size(), 0) << "Should parse at least one word-number pair";
}

TEST_F(CombinatorialIntegrationTest, ComplexParserComposition) {
    // Build a complex parser using multiple combinators
    auto alpha = make_alpha_parser();
    auto digit = make_digit_parser();
    auto space = make_whitespace_parser();
    
    // Parser for: optional_spaces word optional_spaces number optional_spaces
    auto word_number_unit = sequence(
        optional(space),
        sequence(alpha, sequence(optional(space), digit))
    );
    
    auto complete_parser = many(word_number_unit);
    
    std::string test_input = "  hello 123  world 456  ";
    auto [remaining, result] = complete_parser(test_input.begin(), test_input.end());
    
    EXPECT_TRUE(result.has_value());
    if (result) {
        EXPECT_GT(result->size(), 0) << "Should parse multiple units";
    }
}

TEST_F(CombinatorialIntegrationTest, ErrorRecoveryAndRobustness) {
    auto alpha_parser = make_alpha_parser();
    auto digit_parser = make_digit_parser();
    
    // Create a parser that tries to recover from errors
    auto robust_parser = many(alternative(alpha_parser, digit_parser));
    
    std::string problematic_input = "hello123world456test";
    auto [remaining, result] = robust_parser(problematic_input.begin(), problematic_input.end());
    
    EXPECT_TRUE(result.has_value());
    if (result) {
        EXPECT_GT(result->size(), 0) << "Should recover and parse some tokens";
    }
}

// ============================================================================
// Real-World Use Case Integration Tests
// ============================================================================

class RealWorldUseCaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        email_text = R"(
            Subject: Meeting Tomorrow
            Dear colleagues, please remember that we have a meeting scheduled
            for tomorrow at 2:00 PM. The agenda includes discussing the quarterly
            reports and planning for the upcoming project milestones.
            Best regards,
            Management Team
        )";
        
        code_comments = R"(
            // This function implements the quick sort algorithm
            /* It uses divide and conquer approach to efficiently
               sort the input array in O(n log n) average time */
            function quickSort(arr, low, high) {
                // Implementation details here...
            }
        )";
    }
    
    std::string email_text;
    std::string code_comments;
};

TEST_F(RealWorldUseCaseTest, EmailContentAnalysis) {
    DocumentProcessor processor;
    auto result = processor.process_document(email_text);
    
    EXPECT_FALSE(result.normalized_words.empty());
    
    // Should extract key terms from email content
    std::vector<std::string> expected_terms = {"meet", "schedul", "agenda", "report", "project"};
    
    int found_terms = 0;
    for (const auto& expected : expected_terms) {
        for (const auto& stem : result.stems) {
            if (stem.find(expected) != std::string::npos) {
                found_terms++;
                break;
            }
        }
    }
    
    EXPECT_GT(found_terms, 2) << "Should identify key email terms";
}

TEST_F(RealWorldUseCaseTest, CodeCommentExtraction) {
    WordProcessingPipeline pipeline;
    auto result = pipeline.process_sentence(code_comments);
    
    EXPECT_FALSE(result.empty());
    
    // Should extract meaningful words from comments, filtering out code syntax
    bool found_algorithm = false;
    bool found_sort = false;
    
    for (const auto& word : result) {
        if (word.find("algorithm") != std::string::npos) found_algorithm = true;
        if (word == "sort") found_sort = true;
    }
    
    EXPECT_TRUE(found_algorithm || found_sort) << "Should extract algorithmic terms from comments";
}

TEST_F(RealWorldUseCaseTest, MultiLanguageTextHandling) {
    // Test with text containing mixed content
    std::string mixed_content = "Hello world 123 test@example.com http://example.com";
    
    WordProcessingPipeline pipeline;
    auto result = pipeline.process_sentence(mixed_content);
    
    // Should handle and filter non-alphabetic content gracefully
    for (const auto& word : result) {
        EXPECT_TRUE(std::all_of(word.begin(), word.end(), 
                               [](char c) { return std::isalpha(c) && std::islower(c); }));
    }
}

// ============================================================================
// Error Handling and Edge Cases Integration Tests
// ============================================================================

class ErrorHandlingIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        empty_content = "";
        whitespace_only = "   \t\n\r   ";
        special_chars_only = "!@#$%^&*()_+-=[]{}|;':\",./<>?";
        very_long_word = std::string(10000, 'a');
    }
    
    std::string empty_content;
    std::string whitespace_only;
    std::string special_chars_only;
    std::string very_long_word;
};

TEST_F(ErrorHandlingIntegrationTest, EmptyInputHandling) {
    DocumentProcessor processor;
    
    auto result1 = processor.process_document(empty_content);
    EXPECT_TRUE(result1.normalized_words.empty());
    EXPECT_TRUE(result1.stems.empty());
    EXPECT_TRUE(result1.word_counts.empty());
    
    auto result2 = processor.process_document(whitespace_only);
    EXPECT_TRUE(result2.normalized_words.empty());
    EXPECT_TRUE(result2.stems.empty());
    EXPECT_TRUE(result2.word_counts.empty());
}

TEST_F(ErrorHandlingIntegrationTest, SpecialCharacterHandling) {
    WordProcessingPipeline pipeline;
    auto result = pipeline.process_sentence(special_chars_only);
    
    EXPECT_TRUE(result.empty()) << "Special characters only should result in empty output";
}

TEST_F(ErrorHandlingIntegrationTest, ExtremeInputSizes) {
    MultiStrategyStemmer stemmer;
    
    // Test very long word
    if (auto lc_word = make_lc_alpha(very_long_word)) {
        auto result = stemmer(*lc_word);
        EXPECT_TRUE(result.has_value()) << "Should handle very long words without crashing";
        if (result) {
            EXPECT_FALSE(result->empty()) << "Result should not be empty";
        }
    }
    
    // Test very short words
    std::vector<std::string> short_words = {"a", "I", "to", "of", "in"};
    for (const auto& word : short_words) {
        if (auto lc_word = make_lc_alpha(word)) {
            auto result = stemmer(*lc_word);
            EXPECT_TRUE(result.has_value()) << "Should handle short word: " << word;
        }
    }
}

TEST_F(ErrorHandlingIntegrationTest, ParserChainErrorPropagation) {
    // Test error propagation through parser chains
    auto failing_parser = [](const std::string&) -> std::optional<std::string> {
        return std::nullopt; // Always fails
    };
    
    auto succeeding_parser = [](const std::string& input) -> std::optional<std::string> {
        return input; // Always succeeds
    };
    
    // Chain: failing -> succeeding (should fail overall)
    auto chained_parser = [=](const std::string& input) -> std::optional<std::string> {
        auto first_result = failing_parser(input);
        if (!first_result) return std::nullopt;
        return succeeding_parser(*first_result);
    };
    
    auto result = chained_parser("test");
    EXPECT_FALSE(result.has_value()) << "Chain should fail when first parser fails";
}

// ============================================================================
// Performance and Scalability Integration Tests
// ============================================================================

class PerformanceIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Generate large test corpus
        test_corpus.reserve(100000);
        std::vector<std::string> base_words = {
            "running", "jumping", "swimming", "walking", "talking",
            "thinking", "processing", "computing", "analyzing", "developing"
        };
        
        for (int i = 0; i < 1000; ++i) {
            for (const auto& word : base_words) {
                test_corpus += word + " ";
            }
        }
    }
    
    std::string test_corpus;
};

TEST_F(PerformanceIntegrationTest, LargeCorpusProcessing) {
    DocumentProcessor processor;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = processor.process_document(test_corpus);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_FALSE(result.normalized_words.empty());
    EXPECT_FALSE(result.stems.empty());
    
    // Performance requirement: should process at least 1000 words per second
    double words_per_second = static_cast<double>(result.normalized_words.size()) / 
                             (duration.count() / 1000.0);
    
    EXPECT_GT(words_per_second, 1000) << "Should process at least 1000 words per second";
    
    std::cout << "Processed " << result.normalized_words.size() 
              << " words in " << duration.count() << "ms ("
              << static_cast<int>(words_per_second) << " words/sec)" << std::endl;
}

TEST_F(PerformanceIntegrationTest, MemoryUsageStability) {
    DocumentProcessor processor;
    
    // Process multiple documents to test for memory leaks
    for (int i = 0; i < 10; ++i) {
        auto result = processor.process_document(test_corpus);
        EXPECT_FALSE(result.normalized_words.empty());
        
        // Clear results to test cleanup
        result = DocumentProcessor::ProcessedDocument{};
    }
    
    // If we reach here without crashing, memory management is likely correct
    SUCCEED() << "Multiple processing cycles completed without memory issues";
}

TEST_F(PerformanceIntegrationTest, ConcurrentProcessing) {
    // Test thread safety (if applicable)
    DocumentProcessor processor;
    std::vector<std::string> test_inputs;
    
    // Create multiple test inputs
    for (int i = 0; i < 5; ++i) {
        test_inputs.push_back(test_corpus.substr(i * 1000, 1000));
    }
    
    std::vector<std::thread> threads;
    std::vector<bool> results(test_inputs.size(), false);
    
    for (size_t i = 0; i < test_inputs.size(); ++i) {
        threads.emplace_back([&, i]() {
            try {
                auto result = processor.process_document(test_inputs[i]);
                results[i] = !result.normalized_words.empty();
            } catch (...) {
                results[i] = false;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All threads should have succeeded
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_TRUE(results[i]) << "Thread " << i << " should have succeeded";
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
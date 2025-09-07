#pragma once

/**
 * @file composition_examples.hpp
 * @brief Concrete examples demonstrating algebraic parser composition patterns
 * 
 * This file provides practical examples of how to compose simple algebraic parsers
 * into sophisticated parsing systems, building on the theoretical framework.
 */

#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include "parsers/fsm_string_rewriter.hpp"
#include <variant>
#include <vector>
#include <functional>
#include <optional>
#include <string>
#include <algorithm>
#include <numeric>

namespace alga::examples {

    using namespace alga;

    // ============================================================================
    // Example 1: Basic Parser Composition - Sequential Processing
    // ============================================================================
    
    /**
     * Demonstrates how to chain parsers sequentially, where each parser
     * transforms the output of the previous one. This models function composition
     * in the category of parsers.
     */
    template<typename Parser1, typename Parser2>
    class SequentialComposition {
    private:
        Parser1 first_parser;
        Parser2 second_parser;
        
    public:
        using input_type = typename Parser1::input_type;
        using output_type = typename Parser2::output_type;
        
        SequentialComposition(Parser1 p1, Parser2 p2) 
            : first_parser(std::move(p1)), second_parser(std::move(p2)) {}
        
        auto operator()(input_type const& input) const -> std::optional<output_type> {
            auto intermediate = first_parser(input);
            if (!intermediate) return std::nullopt;
            return second_parser(*intermediate);
        }
    };
    
    // Factory function for easy composition
    template<typename P1, typename P2>
    auto compose(P1&& p1, P2&& p2) {
        return SequentialComposition<std::decay_t<P1>, std::decay_t<P2>>{
            std::forward<P1>(p1), std::forward<P2>(p2)
        };
    }

    // ============================================================================
    // Example 2: Alternative Parser - Choice Combinator
    // ============================================================================
    
    /**
     * Implements the choice combinator (|) that tries the first parser,
     * and if it fails, tries the second parser. This models sum types
     * in the algebraic structure.
     */
    template<typename Parser1, typename Parser2>
    class AlternativeParser {
    private:
        Parser1 first_choice;
        Parser2 second_choice;
        
    public:
        using input_type = typename Parser1::input_type;
        using output_type = std::variant<typename Parser1::output_type, typename Parser2::output_type>;
        
        static_assert(std::is_same_v<typename Parser1::input_type, typename Parser2::input_type>,
                     "Both parsers must have the same input type");
        
        AlternativeParser(Parser1 p1, Parser2 p2)
            : first_choice(std::move(p1)), second_choice(std::move(p2)) {}
        
        auto operator()(input_type const& input) const -> std::optional<output_type> {
            if (auto result1 = first_choice(input)) {
                return output_type{*result1};
            }
            if (auto result2 = second_choice(input)) {
                return output_type{*result2};
            }
            return std::nullopt;
        }
    };
    
    template<typename P1, typename P2>
    auto alternative(P1&& p1, P2&& p2) {
        return AlternativeParser<std::decay_t<P1>, std::decay_t<P2>>{
            std::forward<P1>(p1), std::forward<P2>(p2)
        };
    }

    // ============================================================================
    // Example 3: Practical Word Processing Pipeline
    // ============================================================================
    
    /**
     * A comprehensive example that combines multiple algebraic parsers
     * to create a sophisticated word processing system.
     */
    class WordProcessingPipeline {
    private:
        porter2_stemmer stemmer;
        fsm_string_rewriter normalizer;
        
    public:
        WordProcessingPipeline() {
            // Setup normalization rules
            normalizer.push("\\s+", " ");           // normalize whitespace
            normalizer.push("^\\s+|\\s+$", "");     // trim
            normalizer.push("[^a-zA-Z\\s]", "");    // remove non-alphabetic
        }
        
        /**
         * Process a sentence into normalized stems with fallback handling
         */
        auto process_sentence(std::string const& sentence) const -> std::vector<std::string> {
            // Step 1: Normalize the input
            auto normalized = normalizer(sentence);
            
            // Step 2: Split into words
            auto words = split_words(normalized);
            
            // Step 3: Process each word through the algebraic pipeline
            std::vector<std::string> results;
            results.reserve(words.size());
            
            for (auto const& word : words) {
                auto processed = process_single_word(word);
                if (processed) {
                    results.push_back(*processed);
                }
            }
            
            return results;
        }
        
    private:
        auto split_words(std::string const& text) const -> std::vector<std::string> {
            std::vector<std::string> words;
            std::string current_word;
            
            for (char c : text) {
                if (std::isspace(c)) {
                    if (!current_word.empty()) {
                        words.push_back(std::move(current_word));
                        current_word.clear();
                    }
                } else {
                    current_word += c;
                }
            }
            
            if (!current_word.empty()) {
                words.push_back(std::move(current_word));
            }
            
            return words;
        }
        
        auto process_single_word(std::string const& word) const -> std::optional<std::string> {
            // Try to create lc_alpha (validates and normalizes)
            auto lc_word = make_lc_alpha(word);
            if (!lc_word) return std::nullopt;
            
            // Apply stemming to the valid lc_alpha word
            auto stem_result = stemmer(*lc_word);
            if (!stem_result) return std::nullopt;
            return static_cast<std::string>(*stem_result);
        }
    };

    // ============================================================================
    // Example 4: Monadic Parser Composition
    // ============================================================================
    
    /**
     * Demonstrates monadic composition where the result of one parser
     * determines which parser to apply next. This enables context-sensitive
     * parsing within the algebraic framework.
     */
    template<typename Parser, typename Function>
    class MonadicParser {
    private:
        Parser base_parser;
        Function continuation;
        
    public:
        using input_type = typename Parser::input_type;
        using output_type = typename std::invoke_result_t<Function, typename Parser::output_type>::output_type;
        
        MonadicParser(Parser p, Function f) 
            : base_parser(std::move(p)), continuation(std::move(f)) {}
        
        auto operator()(input_type const& input) const -> std::optional<output_type> {
            auto first_result = base_parser(input);
            if (!first_result) return std::nullopt;
            
            auto next_parser = continuation(*first_result);
            return next_parser(input);
        }
    };
    
    template<typename P, typename F>
    auto bind(P&& parser, F&& func) {
        return MonadicParser<std::decay_t<P>, std::decay_t<F>>{
            std::forward<P>(parser), std::forward<F>(func)
        };
    }

    // ============================================================================
    // Example 5: Multi-Strategy Stemming with Algebraic Fallbacks
    // ============================================================================
    
    /**
     * An advanced example showing how to combine multiple stemming strategies
     * in an algebraic way, with well-defined fallback behavior.
     */
    class MultiStrategyStemmer {
    public:
        using input_type = lc_alpha;
        using output_type = std::string;
        
        /**
         * Applies multiple stemming strategies in order, using the first
         * successful result. Demonstrates algebraic choice composition.
         */
        auto operator()(lc_alpha const& word) const -> std::optional<output_type> {
            // Strategy 1: Porter2 stemming
            if (auto porter_result = try_porter2(word)) {
                return std::string{*porter_result};
            }
            
            // Strategy 2: Simple suffix removal (fallback)
            if (auto suffix_result = try_suffix_removal(word)) {
                return *suffix_result;
            }
            
            // Strategy 3: Return original word (final fallback)
            return std::string{word};
        }
        
    private:
        auto try_porter2(lc_alpha const& word) const -> std::optional<porter2_stem> {
            try {
                porter2_stemmer stemmer;
                return stemmer(word);
            } catch (...) {
                return std::nullopt;
            }
        }
        
        auto try_suffix_removal(lc_alpha const& word) const -> std::optional<std::string> {
            auto word_str = std::string{word};
            
            // Simple suffix removal rules
            std::vector<std::string> suffixes = {"ing", "ed", "er", "est", "ly", "s"};
            
            for (auto const& suffix : suffixes) {
                if (word_str.length() > suffix.length() + 2) { // Keep meaningful stem
                    if (word_str.substr(word_str.length() - suffix.length()) == suffix) {
                        return word_str.substr(0, word_str.length() - suffix.length());
                    }
                }
            }
            
            return std::nullopt;
        }
    };

    // ============================================================================
    // Example 6: Frequency-Based Parsing with Statistical Composition
    // ============================================================================
    
    /**
     * Demonstrates how to incorporate statistical information into
     * algebraic parser composition, maintaining mathematical rigor
     * while adding practical utility.
     */
    class StatisticalWordParser {
    private:
        std::map<std::string, double> word_frequencies;
        double frequency_threshold;
        
    public:
        StatisticalWordParser(double threshold = 0.001) 
            : frequency_threshold(threshold) {
            // In a real implementation, these would be loaded from a corpus
            word_frequencies["the"] = 0.0687;
            word_frequencies["run"] = 0.0023;
            word_frequencies["running"] = 0.0012;
            // ... more frequencies
        }
        
        /**
         * Parses words with frequency weighting, demonstrating how
         * external information can be incorporated algebraically.
         */
        auto parse_with_frequency(std::string const& word) const 
            -> std::optional<std::pair<std::string, double>> {
            
            auto lc_word = make_lc_alpha(word);
            if (!lc_word) return std::nullopt;
            
            auto word_str = std::string{*lc_word};
            
            // Direct frequency lookup
            if (auto it = word_frequencies.find(word_str); 
                it != word_frequencies.end() && it->second >= frequency_threshold) {
                return std::make_pair(word_str, it->second);
            }
            
            // Try stemmed version
            porter2_stemmer stemmer;
            if (auto stem = stemmer(*lc_word)) {
                auto stem_str = std::string{*stem};
                if (auto it = word_frequencies.find(stem_str);
                    it != word_frequencies.end() && it->second >= frequency_threshold) {
                    return std::make_pair(stem_str, it->second);
                }
            }
            
            return std::nullopt;
        }
        
        /**
         * Compose with other parsers using frequency as a weighting mechanism
         */
        template<typename OtherParser>
        auto weighted_alternative(OtherParser&& other) const {
            return [*this, other = std::forward<OtherParser>(other)](std::string const& input) {
                auto freq_result = parse_with_frequency(input);
                auto other_result = other(input);
                
                if (freq_result && other_result) {
                    // Choose based on frequency weight
                    return freq_result->second > 0.01 ? 
                        std::optional{freq_result->first} : other_result;
                } else if (freq_result) {
                    return std::optional{freq_result->first};
                } else {
                    return other_result;
                }
            };
        }
    };

    // ============================================================================
    // Example 7: Complete Document Processing System
    // ============================================================================
    
    /**
     * A comprehensive example that demonstrates the full power of algebraic
     * parser composition by building a complete document processing system
     * from simple, composable parts.
     */
    class DocumentProcessor {
    private:
        WordProcessingPipeline word_processor;
        MultiStrategyStemmer multi_stemmer;
        StatisticalWordParser statistical_parser;
        
    public:
        struct ProcessedDocument {
            std::vector<std::string> normalized_words;
            std::vector<std::string> stems;
            std::map<std::string, int> word_counts;
            std::map<std::string, double> statistical_scores;
        };
        
        auto process_document(std::string const& document) const -> ProcessedDocument {
            ProcessedDocument result;
            
            // Step 1: Basic word processing
            result.normalized_words = word_processor.process_sentence(document);
            
            // Step 2: Apply multi-strategy stemming
            result.stems.reserve(result.normalized_words.size());
            for (auto const& word : result.normalized_words) {
                if (auto lc = make_lc_alpha(word)) {
                    if (auto stem = multi_stemmer(*lc)) {
                        result.stems.push_back(*stem);
                    }
                }
            }
            
            // Step 3: Count frequencies
            for (auto const& stem : result.stems) {
                result.word_counts[stem]++;
            }
            
            // Step 4: Apply statistical analysis
            for (auto const& word : result.normalized_words) {
                if (auto stat_result = statistical_parser.parse_with_frequency(word)) {
                    result.statistical_scores[word] = stat_result->second;
                }
            }
            
            return result;
        }
        
        /**
         * Demonstrates composing the entire system as a single parser
         */
        auto as_composed_parser() const {
            return [*this](std::string const& input) -> std::optional<ProcessedDocument> {
                try {
                    return process_document(input);
                } catch (...) {
                    return std::nullopt;
                }
            };
        }
    };

    // ============================================================================
    // Utility Functions for Testing and Demonstration
    // ============================================================================
    
    /**
     * Utility function to demonstrate the algebraic properties of parser composition
     */
    template<typename Parser1, typename Parser2, typename Parser3>
    void demonstrate_associativity(Parser1&& p1, Parser2&& p2, Parser3&& p3, 
                                 std::string const& test_input) {
        // Test that (p1 ∘ p2) ∘ p3 = p1 ∘ (p2 ∘ p3)
        auto left_associated = compose(compose(p1, p2), p3);
        auto right_associated = compose(p1, compose(p2, p3));
        
        auto result1 = left_associated(test_input);
        auto result2 = right_associated(test_input);
        
        std::cout << "Associativity test: " 
                  << (result1 == result2 ? "PASSED" : "FAILED") << std::endl;
    }
    
    /**
     * Factory functions for common parser patterns
     */
    namespace factories {
        
        auto make_stemming_parser() {
            return porter2_stemmer{};
        }
        
        auto make_normalizing_parser() {
            fsm_string_rewriter rewriter;
            rewriter.push("\\s+", " ");
            rewriter.push("^\\s+|\\s+$", "");
            return rewriter;
        }
        
        auto make_fallback_stemmer() {
            return alternative(
                make_stemming_parser(),
                [](lc_alpha const& word) -> std::optional<std::string> {
                    return std::string{word}; // identity fallback
                }
            );
        }
        
        template<typename... Parsers>
        auto make_choice_chain(Parsers&&... parsers) {
            return (alternative(parsers) | ...);
        }
    }

} // namespace alga::examples
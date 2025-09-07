/**
 * @file performance_benchmark.cpp
 * @brief Performance benchmarking framework for algebraic parsers
 * 
 * This suite benchmarks various aspects of the algebraic parser library:
 * - Template instantiation overhead
 * - Runtime performance of algebraic operations
 * - Memory usage patterns
 * - Scaling behavior with input size
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <memory>
#include <sstream>
#include <iomanip>
#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include "parsers/combinatorial_parser_fixed.hpp"

using namespace alga;
using namespace alga::combinatorial;

class PerformanceBenchmark {
private:
    std::mt19937 gen;
    
    // Timing utilities
    template<typename F>
    std::chrono::nanoseconds time_function(F&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    }
    
    template<typename F>
    void benchmark_function(const std::string& name, F&& func, size_t iterations = 10000) {
        std::cout << "Benchmarking: " << name << "\n";
        
        // Warmup
        for (size_t i = 0; i < iterations / 10; ++i) {
            func();
        }
        
        // Actual measurement
        std::vector<std::chrono::nanoseconds> times;
        times.reserve(iterations);
        
        for (size_t i = 0; i < iterations; ++i) {
            times.push_back(time_function(func));
        }
        
        // Calculate statistics
        std::sort(times.begin(), times.end());
        
        auto min_time = times.front();
        auto max_time = times.back();
        auto median_time = times[times.size() / 2];
        
        auto total_time = std::accumulate(times.begin(), times.end(), 
                                         std::chrono::nanoseconds{0});
        auto avg_time = total_time / iterations;
        
        std::cout << "  Iterations: " << iterations << "\n";
        std::cout << "  Min:    " << std::setw(8) << min_time.count() << " ns\n";
        std::cout << "  Avg:    " << std::setw(8) << avg_time.count() << " ns\n";
        std::cout << "  Median: " << std::setw(8) << median_time.count() << " ns\n";
        std::cout << "  Max:    " << std::setw(8) << max_time.count() << " ns\n";
        std::cout << "  Total:  " << std::setw(8) << (total_time.count() / 1000000) << " ms\n\n";
    }
    
    // Test data generators
    std::string generate_alpha_string(size_t length) {
        std::uniform_int_distribution<> dist('a', 'z');
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            result += static_cast<char>(dist(gen));
        }
        return result;
    }
    
    std::vector<std::string> generate_word_list(size_t count, size_t avg_length = 8) {
        std::vector<std::string> words;
        words.reserve(count);
        
        std::uniform_int_distribution<> len_dist(3, avg_length * 2);
        
        for (size_t i = 0; i < count; ++i) {
            words.push_back(generate_alpha_string(len_dist(gen)));
        }
        
        return words;
    }
    
public:
    PerformanceBenchmark() : gen(std::random_device{}()) {}
    
    // ============================================================================
    // Algebraic Operations Benchmarks
    // ============================================================================
    
    void benchmark_lc_alpha_operations() {
        std::cout << "=== lc_alpha Algebraic Operations ===\n";
        
        // Generate test data
        std::vector<std::string> test_strings = generate_word_list(1000, 10);
        std::vector<lc_alpha> test_alphas;
        
        for (const auto& s : test_strings) {
            if (auto alpha = make_lc_alpha(s)) {
                test_alphas.push_back(*alpha);
            }
        }
        
        // Benchmark lc_alpha creation
        size_t creation_index = 0;
        benchmark_function("lc_alpha creation", [&]() {
            auto result = make_lc_alpha(test_strings[creation_index % test_strings.size()]);
            creation_index++;
        });
        
        // Benchmark lc_alpha concatenation (monoid operation)
        size_t concat_index = 0;
        benchmark_function("lc_alpha concatenation", [&]() {
            if (test_alphas.size() >= 2) {
                size_t i = concat_index % (test_alphas.size() - 1);
                auto result = test_alphas[i] * test_alphas[i + 1];
                concat_index++;
            }
        });
        
        // Benchmark lc_alpha comparison
        size_t comp_index = 0;
        benchmark_function("lc_alpha comparison", [&]() {
            if (test_alphas.size() >= 2) {
                size_t i = comp_index % (test_alphas.size() - 1);
                volatile bool result = (test_alphas[i] == test_alphas[i + 1]);
                comp_index++;
            }
        });
        
        // Benchmark string conversion
        size_t conv_index = 0;
        benchmark_function("lc_alpha string conversion", [&]() {
            if (!test_alphas.empty()) {
                std::string result = (std::string)test_alphas[conv_index % test_alphas.size()];
                conv_index++;
            }
        });
    }
    
    // ============================================================================
    // Porter2 Stemmer Benchmarks
    // ============================================================================
    
    void benchmark_porter2_stemmer() {
        std::cout << "=== Porter2 Stemmer Performance ===\n";
        
        // Generate test data with various word lengths
        std::vector<std::string> short_words = generate_word_list(1000, 5);
        std::vector<std::string> medium_words = generate_word_list(1000, 10);
        std::vector<std::string> long_words = generate_word_list(1000, 20);
        
        porter2_stemmer stemmer;
        
        // Benchmark short words
        size_t short_index = 0;
        benchmark_function("Porter2 stemming (short words)", [&]() {
            auto result = stemmer(short_words[short_index % short_words.size()]);
            short_index++;
        });
        
        // Benchmark medium words
        size_t medium_index = 0;
        benchmark_function("Porter2 stemming (medium words)", [&]() {
            auto result = stemmer(medium_words[medium_index % medium_words.size()]);
            medium_index++;
        });
        
        // Benchmark long words
        size_t long_index = 0;
        benchmark_function("Porter2 stemming (long words)", [&]() {
            auto result = stemmer(long_words[long_index % long_words.size()]);
            long_index++;
        });
        
        // Benchmark with lc_alpha input
        std::vector<lc_alpha> alpha_words;
        for (const auto& word : medium_words) {
            if (auto alpha = make_lc_alpha(word)) {
                alpha_words.push_back(*alpha);
            }
        }
        
        size_t alpha_index = 0;
        benchmark_function("Porter2 stemming (lc_alpha input)", [&]() {
            if (!alpha_words.empty()) {
                auto result = stemmer(alpha_words[alpha_index % alpha_words.size()]);
                alpha_index++;
            }
        });
    }
    
    // ============================================================================
    // Parser Combinator Benchmarks
    // ============================================================================
    
    void benchmark_parser_combinators() {
        std::cout << "=== Parser Combinator Performance ===\n";
        
        // Generate test input
        std::ostringstream input_stream;
        for (int i = 0; i < 100; ++i) {
            input_stream << "word" << i << " 123 ";
        }
        std::string large_input = input_stream.str();
        
        // Basic parser benchmarks
        auto alpha = make_alpha_parser();
        auto digit = make_digit_parser();
        auto ws = make_whitespace_parser();
        
        size_t parse_index = 0;
        benchmark_function("Alpha parser", [&]() {
            size_t start_pos = (parse_index * 20) % (large_input.size() - 10);
            auto begin = large_input.begin() + start_pos;
            auto end = large_input.end();
            auto [remaining, result] = alpha(begin, end);
            parse_index++;
        }, 50000);
        
        // Sequence parser benchmark
        auto seq_parser = sequence(alpha, ws);
        size_t seq_index = 0;
        benchmark_function("Sequence parser (alpha + whitespace)", [&]() {
            size_t start_pos = (seq_index * 20) % (large_input.size() - 10);
            auto begin = large_input.begin() + start_pos;
            auto end = large_input.end();
            auto [remaining, result] = seq_parser(begin, end);
            seq_index++;
        }, 20000);
        
        // Many parser benchmark
        auto many_alpha = many(alpha);
        benchmark_function("Many parser (alpha*)", [&]() {
            auto [remaining, result] = many_alpha(large_input.begin(), large_input.end());
        }, 1000);
        
        // Complex nested parser
        auto complex_parser = many(sequence(alpha, sequence(ws, digit)));
        benchmark_function("Complex parser (alpha ws digit)*", [&]() {
            auto [remaining, result] = complex_parser(large_input.begin(), large_input.end());
        }, 1000);
    }
    
    // ============================================================================
    // Memory Usage Benchmarks
    // ============================================================================
    
    void benchmark_memory_usage() {
        std::cout << "=== Memory Usage Analysis ===\n";
        
        // Test lc_alpha memory usage
        const size_t test_count = 10000;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        {
            std::vector<lc_alpha> alphas;
            alphas.reserve(test_count);
            
            for (size_t i = 0; i < test_count; ++i) {
                std::string word = "test" + std::to_string(i);
                if (auto alpha = make_lc_alpha(word)) {
                    alphas.push_back(*alpha);
                }
            }
            
            // Perform operations to ensure no optimization
            size_t total_length = 0;
            for (const auto& alpha : alphas) {
                total_length += alpha.size();
            }
            
            volatile size_t result = total_length; // Prevent optimization
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Created and processed " << test_count << " lc_alpha objects in " 
                  << duration.count() << "ms\n";
        
        // Test porter2_stem memory usage
        start_time = std::chrono::high_resolution_clock::now();
        
        {
            porter2_stemmer stemmer;
            std::vector<porter2_stem> stems;
            stems.reserve(test_count);
            
            for (size_t i = 0; i < test_count; ++i) {
                std::string word = generate_alpha_string(8 + (i % 10));
                if (auto alpha = make_lc_alpha(word)) {
                    stems.push_back(stemmer(*alpha));
                }
            }
            
            // Perform operations
            size_t total_length = 0;
            for (const auto& stem : stems) {
                total_length += std::string(stem).size();
            }
            
            volatile size_t result = total_length;
        }
        
        end_time = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Created and processed " << test_count << " porter2_stem objects in " 
                  << duration.count() << "ms\n\n";
    }
    
    // ============================================================================
    // Scaling Benchmarks
    // ============================================================================
    
    void benchmark_scaling() {
        std::cout << "=== Scaling Analysis ===\n";
        
        std::vector<size_t> input_sizes = {100, 500, 1000, 2000, 5000, 10000};
        
        for (size_t size : input_sizes) {
            std::cout << "Input size: " << size << " words\n";
            
            // Generate input
            std::vector<std::string> words = generate_word_list(size, 8);
            
            // Test lc_alpha creation scaling
            auto start = std::chrono::high_resolution_clock::now();
            
            std::vector<lc_alpha> alphas;
            alphas.reserve(size);
            
            for (const auto& word : words) {
                if (auto alpha = make_lc_alpha(word)) {
                    alphas.push_back(*alpha);
                }
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            std::cout << "  lc_alpha creation: " << duration.count() 
                      << " μs (" << (duration.count() / static_cast<double>(size)) << " μs/word)\n";
            
            // Test stemming scaling
            start = std::chrono::high_resolution_clock::now();
            
            porter2_stemmer stemmer;
            std::vector<porter2_stem> stems;
            stems.reserve(size);
            
            for (const auto& alpha : alphas) {
                stems.push_back(stemmer(alpha));
            }
            
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            std::cout << "  Porter2 stemming: " << duration.count() 
                      << " μs (" << (duration.count() / static_cast<double>(size)) << " μs/word)\n";
            
            // Test monoid operations scaling
            start = std::chrono::high_resolution_clock::now();
            
            size_t total_ops = 0;
            for (size_t i = 1; i < alphas.size() && i < 100; ++i) { // Limit to avoid huge strings
                auto temp_result = alphas[0] * alphas[i];
                volatile auto result_size = temp_result.size(); // Prevent optimization
                total_ops++;
            }
            
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            std::cout << "  Monoid concatenation: " << duration.count() << " μs\n\n";
        }
    }
    
    // ============================================================================
    // Template Instantiation Analysis
    // ============================================================================
    
    void benchmark_template_instantiation() {
        std::cout << "=== Template Instantiation Analysis ===\n";
        
        // This section analyzes compile-time costs by measuring runtime
        // patterns that correlate with template complexity
        
        // Simple template usage
        benchmark_function("Simple parser instantiation", []() {
            auto parser = make_alpha_parser();
            volatile auto* ptr = &parser; // Prevent optimization
        }, 100000);
        
        // Complex template composition
        benchmark_function("Complex parser composition", []() {
            auto alpha = make_alpha_parser();
            auto digit = make_digit_parser();
            auto ws = make_whitespace_parser();
            
            auto complex = sequence(
                sequence(alpha, ws),
                sequence(digit, maybe(ws))
            );
            
            volatile auto* ptr = &complex;
        }, 50000);
        
        // Nested template structures
        benchmark_function("Deeply nested templates", []() {
            auto base = make_alpha_parser();
            auto nested = maybe(maybe(maybe(maybe(base))));
            volatile auto* ptr = &nested;
        }, 50000);
    }
    
    // ============================================================================
    // Main Benchmark Runner
    // ============================================================================
    
    void run_all_benchmarks() {
        std::cout << "====================================\n";
        std::cout << "  Algebraic Parsers Benchmark Suite\n";
        std::cout << "====================================\n\n";
        
        benchmark_lc_alpha_operations();
        benchmark_porter2_stemmer();
        benchmark_parser_combinators();
        benchmark_memory_usage();
        benchmark_scaling();
        benchmark_template_instantiation();
        
        std::cout << "====================================\n";
        std::cout << "  Benchmark Suite Complete\n";
        std::cout << "====================================\n";
    }
};

int main() {
    PerformanceBenchmark benchmark;
    benchmark.run_all_benchmarks();
    return 0;
}
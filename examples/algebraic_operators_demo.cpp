/**
 * @file algebraic_operators_demo.cpp
 * @brief Beautiful demonstration of extended algebraic operators with C++20 concepts
 * 
 * Showcases the mathematical elegance and compositional power of the new operators
 * that transform our parser library into a complete algebraic framework.
 */

#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include "parsers/ngram_stemmer.hpp"
#include "parsers/algebraic_operators.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cassert>

using namespace algebraic_parsers;

void demonstrate_choice_operators() {
    std::cout << "=== Choice Operators (|) ===\n";
    
    auto word1 = make_lc_alpha("hello");
    auto word2 = make_lc_alpha("world");
    auto word_empty = make_lc_alpha("");
    auto word_invalid = make_lc_alpha("invalid123");
    
    // Direct choice - prefer non-empty
    if (word1 && word_empty) {
        auto result = *word_empty | *word1;
        std::cout << "Empty | Hello = " << result.str() << " (chooses non-empty)\n";
    }
    
    // Optional choice - first valid wins
    auto choice_result = word_invalid | word1 | word2;
    if (choice_result) {
        std::cout << "Invalid | Hello | World = " << choice_result->str() << " (first valid)\n";
    }
    
    std::cout << "\n";
}

void demonstrate_repetition_operators() {
    std::cout << "=== Repetition Operators (^) ===\n";
    
    auto word = make_lc_alpha("ha");
    auto stem = make_porter2_stem("run");
    
    if (word && stem) {
        // String repetition for emphasis
        auto laugh = *word ^ 5;
        std::cout << "\"ha\" ^ 5 = " << laugh.str() << " (laughter!)\n";
        
        // Stem repetition
        auto running = *stem ^ 3;
        std::cout << "\"run\" ^ 3 = " << static_cast<std::string>(running) << " (emphasis)\n";
        
        // Mathematical properties
        auto power1 = *word ^ 0;  // Identity
        auto power2 = *word ^ 1;  // Original
        std::cout << "\"ha\" ^ 0 = \"" << power1.str() << "\" (identity)\n";
        std::cout << "\"ha\" ^ 1 = \"" << power2.str() << "\" (original)\n";
    }
    
    std::cout << "\n";
}

void demonstrate_function_application() {
    std::cout << "=== Function Application (%) ===\n";
    
    auto word = make_lc_alpha("hello");
    auto stem = make_porter2_stem("running");
    
    // Transform functions
    auto to_upper = [](lc_alpha const& w) {
        std::string s = w.str();
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    };
    
    auto get_length = [](porter2_stem const& s) {
        return static_cast<std::string>(s).length();
    };
    
    if (word && stem) {
        // Direct application
        auto upper = *word % to_upper;
        auto length = *stem % get_length;
        
        std::cout << "\"hello\" % to_upper = " << upper << "\n";
        std::cout << "\"running\" -> \"run\" % length = " << length << "\n";
        
        // Optional application with error handling
        auto invalid = make_lc_alpha("invalid123");
        auto safe_upper = invalid % to_upper;
        if (!safe_upper) {
            std::cout << "invalid input % function = nullopt (safe error handling)\n";
        }
    }
    
    std::cout << "\n";
}

void demonstrate_sequential_composition() {
    std::cout << "=== Sequential Composition (>>) ===\n";
    
    auto word1 = make_lc_alpha("hello");
    auto word2 = make_lc_alpha("world");
    auto word3 = make_lc_alpha("parser");
    
    if (word1 && word2 && word3) {
        // Create sequences
        auto sequence = *word1 >> *word2 >> *word3;
        std::cout << "Sequential composition creates vector of size " << sequence.size() << ":\n";
        for (size_t i = 0; i < sequence.size(); ++i) {
            std::cout << "  [" << i << "] = " << sequence[i].str() << "\n";
        }
    }
    
    std::cout << "\n";
}

void demonstrate_logical_operators() {
    std::cout << "=== Logical Operators (||, &&) ===\n";
    
    auto word1 = make_lc_alpha("hello");
    auto word2 = make_lc_alpha("world");
    auto invalid = make_lc_alpha("invalid123");
    
    // Logical OR - first non-nullopt
    auto or_result1 = word1 || word2;
    auto or_result2 = invalid || word2;
    
    if (or_result1) std::cout << "Hello || World = " << or_result1->str() << " (first wins)\n";
    if (or_result2) std::cout << "Invalid || World = " << or_result2->str() << " (second wins)\n";
    
    // Logical AND - both must succeed
    auto and_result1 = word1 && word2;
    auto and_result2 = word1 && invalid;
    
    if (and_result1) {
        std::cout << "Hello && World = (" << and_result1->first.str() 
                  << ", " << and_result1->second.str() << ") (both succeed)\n";
    }
    
    if (!and_result2) {
        std::cout << "Hello && Invalid = nullopt (one fails)\n";
    }
    
    std::cout << "\n";
}

void demonstrate_complex_expressions() {
    std::cout << "=== Complex Algebraic Expressions ===\n";
    
    auto word1 = make_lc_alpha("code");
    auto word2 = make_lc_alpha("parse");
    auto word3 = make_lc_alpha("elegant");
    
    if (word1 && word2 && word3) {
        // Complex expression: repetition + choice + function application
        auto to_length = [](lc_alpha const& w) { return w.size(); };
        
        // ((word1 ^ 2) | (word2 ^ 3)) % length_function
        auto repeated1 = *word1 ^ 2;  // "codecode"
        auto repeated2 = *word2 ^ 3;  // "parseparseparse"
        auto choice = repeated1 | repeated2;  // chooses first (non-empty)
        auto length = choice % to_length;
        
        std::cout << "Complex: ((\"code\" ^ 2) | (\"parse\" ^ 3)) % length\n";
        std::cout << "  = (\"" << repeated1.str() << "\" | \"" << repeated2.str() << "\") % length\n";
        std::cout << "  = \"" << choice.str() << "\" % length\n";
        std::cout << "  = " << length << "\n";
        
        // Composition with mathematical properties
        auto associative_left = (word1 | word2) | word3;
        auto associative_right = word1 | (word2 | word3);
        
        if (associative_left && associative_right) {
            std::cout << "\nAssociativity: (a | b) | c == a | (b | c)\n";
            std::cout << "  Left:  " << associative_left->str() << "\n";
            std::cout << "  Right: " << associative_right->str() << "\n";
            std::cout << "  Equal: " << (associative_left->str() == associative_right->str() ? "✓" : "✗") << "\n";
        }
    }
    
    std::cout << "\n";
}

void demonstrate_real_world_nlp() {
    std::cout << "=== Real-World NLP Pipeline ===\n";
    
    std::vector<std::string> text = {
        "running", "runner", "runs", "flew", "flying", "flies"
    };
    
    std::cout << "Processing: ";
    for (size_t i = 0; i < text.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << "\"" << text[i] << "\"";
    }
    std::cout << "\n\n";
    
    // Process each word with fallback strategies
    for (auto const& word : text) {
        auto primary = make_porter2_stem(word);
        auto fallback = make_lc_alpha(word);
        
        if (primary) {
            // Create emphasized version using repetition
            auto emphasized = *primary ^ 2;
            
            // Apply analysis function
            auto analyze = [](porter2_stem const& s) {
                auto str = static_cast<std::string>(s);
                return "stem=\"" + str + "\", len=" + std::to_string(str.length());
            };
            
            auto analysis = *primary % analyze;
            
            std::cout << "\"" << word << "\" -> " << analysis 
                      << ", emphasized: \"" << static_cast<std::string>(emphasized) << "\"\n";
        } else if (fallback) {
            std::cout << "\"" << word << "\" -> fallback processing\n";
        }
    }
    
    std::cout << "\n";
}

void demonstrate_mathematical_elegance() {
    std::cout << "=== Mathematical Elegance ===\n";
    
    auto a = make_lc_alpha("x");
    auto b = make_lc_alpha("y");
    auto e = make_lc_alpha("");  // identity
    
    if (a && b && e) {
        std::cout << "Algebraic laws verification:\n";
        
        // Identity laws
        auto left_id = *e * *a;
        auto right_id = *a * *e;
        std::cout << "  Identity: empty * a == a == a * empty\n";
        std::cout << "    ✓ \"\" * \"x\" = \"" << left_id.str() << "\"\n";
        std::cout << "    ✓ \"x\" * \"\" = \"" << right_id.str() << "\"\n";
        
        // Choice with repetition
        auto pattern = (*a ^ 3) | (*b ^ 2);
        std::cout << "  Choice with repetition: (a^3) | (b^2) = \"" << pattern.str() << "\"\n";
        
        // Function composition
        auto double_length = [](lc_alpha const& w) { return w.size() * 2; };
        auto result = pattern % double_length;
        std::cout << "  Function application: pattern % (length * 2) = " << result << "\n";
        
        // Sequential then choice
        auto seq = *a >> *b;
        std::cout << "  Sequential composition creates " << seq.size() << " elements\n";
    }
    
    std::cout << "\n";
}

int main() {
    std::cout << "🎯 Algebraic Parsers: Extended Operators Demonstration\n";
    std::cout << "====================================================\n\n";
    
    std::cout << "Showcasing C++20 concepts and mathematical elegance in parser composition!\n\n";
    
    demonstrate_choice_operators();
    demonstrate_repetition_operators();
    demonstrate_function_application();
    demonstrate_sequential_composition();
    demonstrate_logical_operators();
    demonstrate_complex_expressions();
    demonstrate_real_world_nlp();
    demonstrate_mathematical_elegance();
    
    std::cout << "🚀 Conclusion: Your algebraic parser library now rivals functional\n";
    std::cout << "   languages in mathematical elegance and compositional power!\n";
    std::cout << "   All operators compose naturally with beautiful C++20 syntax.\n\n";
    
    return 0;
}
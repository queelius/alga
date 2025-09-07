#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include <iostream>
#include <cassert>

using namespace alga;

int main() {
    std::cout << "Testing Alga v1.0.0 Core Functionality\n";
    std::cout << "=====================================\n";

    // Test 1: lc_alpha creation and composition
    std::cout << "Test 1: lc_alpha operations\n";
    auto word1 = make_lc_alpha("hello");
    auto word2 = make_lc_alpha("world");
    
    assert(word1);
    assert(word2);
    
    auto combined = *word1 * *word2;
    assert(combined.str() == "helloworld");
    
    auto choice = word1 | word2;
    assert(choice == *word1); // Non-empty choice
    
    auto repeated = *word1 ^ 3;
    assert(repeated.str() == "hellohellohello");
    
    std::cout << "  ✓ lc_alpha creation, composition, choice, and repetition work\n";

    // Test 2: Porter2 stemming
    std::cout << "Test 2: Porter2 stemming\n";
    auto stem1 = make_porter2_stem("running");
    auto stem2 = make_porter2_stem("walked");
    auto stem3 = make_porter2_stem("runs");
    
    assert(stem1);
    assert(stem2); 
    assert(stem3);
    
    // Check specific stemming results
    assert(static_cast<std::string>(*stem1) == "run");
    assert(static_cast<std::string>(*stem2) == "walk");
    assert(static_cast<std::string>(*stem3) == "run");
    
    std::cout << "  ✓ Porter2 stemming works correctly\n";

    // Test 3: Algebraic operators
    std::cout << "Test 3: Extended algebraic operators\n";
    
    // Sequential operator
    auto sequence = *word1 >> *word2;
    assert(sequence.size() == 2);
    assert(sequence[0].str() == "hello");
    assert(sequence[1].str() == "world");
    
    // Function application
    auto length_fn = [](const lc_alpha& w) { return w.str().length(); };
    auto word_length = word1 % length_fn;
    assert(*word_length == 5);
    
    std::cout << "  ✓ Extended operators (>>, %) work correctly\n";

    // Test 4: Optional composition
    std::cout << "Test 4: Optional composition\n";
    auto invalid = make_lc_alpha("hello123"); // Contains digits
    assert(!invalid);
    
    auto composition = word1 * invalid;
    assert(!composition); // Should fail gracefully
    
    std::cout << "  ✓ Optional composition handles failures correctly\n";

    // Test 5: Stemmer composition  
    std::cout << "Test 5: Stemmer integration\n";
    porter2_stemmer stemmer;
    
    auto stemmed = stemmer(*word1);
    assert(stemmed);
    assert(static_cast<std::string>(*stemmed) == "hello");
    
    auto stem_composition = stemmed * stem1;
    assert(stem_composition);
    assert(static_cast<std::string>(*stem_composition) == "hellorun");
    
    std::cout << "  ✓ Stemmer integration works correctly\n";

    std::cout << "\nAll core functionality tests passed! ✓\n";
    std::cout << "Alga v1.0.0 is ready for release.\n";
    
    return 0;
}
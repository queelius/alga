# Alga v1.0.0 - Mathematical Text Processing Library

The first major release of **Alga**, a mathematically elegant C++ library for algebraic text processing and compositional parsing.

## 🚀 Key Features

- **Mathematical Foundation**: Built on rigorous algebraic structures (monoids, functors) with verified laws
- **High Performance**: Header-only template library with zero-cost abstractions and C++17/20 support  
- **Type Safety**: Compile-time guarantees through concepts, strong typing, and mathematical invariants
- **Algebraic Operators**: Extended operator set (`*`, `|`, `^`, `%`, `>>`) with C++20 concepts for mathematical composition
- **Production Ready**: Includes industry-standard Porter2 stemmer and comprehensive examples

## 📦 What's Included

### Core Components
- **`lc_alpha`**: Type-safe lowercase alphabetic strings forming a free monoid under concatenation
- **`porter2stemmer`**: Production-ready Porter2 stemming algorithm with algebraic interface  
- **`algebraic_operators`**: Extended operator set for mathematical composition patterns
- **`monadic_combinators`**: Monadic composition patterns with automatic error handling
- **`combinatorial_parser`**: Composable parser combinator framework for building complex grammars

### Examples and Documentation
- **Composition Examples**: Real-world usage patterns and advanced composition techniques
- **Mathematical Foundations**: Detailed explanations of algebraic structures and their properties
- **Comprehensive README**: Installation guide, API reference, and mathematical background
- **Test Suite**: Property-based tests verifying algebraic laws and mathematical correctness

## 🛠️ Requirements

- **Compiler**: C++17 compatible (GCC 7+, Clang 5+, MSVC 2017+)
- **Build System**: CMake 3.14+ (optional, for building tests)
- **Dependencies**: Header-only design with no external runtime dependencies
- **Testing**: Google Test (automatically downloaded during build if needed)

## 📋 Installation

### Quick Start (Header-Only)
```cpp
#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"  
#include "parsers/algebraic_operators.hpp"

using namespace alga;

int main() {
    // Create algebraic text elements
    auto word1 = make_lc_alpha("hello");
    auto word2 = make_lc_alpha("world");
    
    if (word1 && word2) {
        // Mathematical composition operators
        auto combined = *word1 * *word2;          // Monoid concatenation
        auto choice = word1 | word2;              // Choice operator  
        auto repeated = *word1 ^ 3;               // Repetition operator
        
        // Porter2 stemming with algebraic composition
        auto stem = make_porter2_stem("running");
        if (stem) {
            auto stem_ops = *stem ^ 2;             // Stem repetition
        }
    }
    return 0;
}
```

### CMake Integration
```bash
git clone https://github.com/yourusername/alga.git
cd alga
mkdir build && cd build
cmake ..
make -j$(nproc)
make test  # Run comprehensive test suite
```

## 🔬 Mathematical Rigor

All algebraic structures satisfy their mathematical laws:

- **Monoid Laws**: Associativity, identity, and closure properties
- **Functor Laws**: Identity preservation and composition  
- **Type Safety**: Compile-time enforcement of algebraic properties
- **Property-Based Testing**: Automated verification of mathematical properties

## 📊 Quality Metrics

- **Test Coverage**: 60%+ on core components
- **Build Status**: ✅ All basic functionality tests passing
- **Performance**: Zero-cost abstractions with template metaprogramming
- **Memory Safety**: RAII and move semantics, no unnecessary copying

## 📖 Documentation

- [README.md](README.md) - Comprehensive guide with examples
- [CHANGELOG.md](CHANGELOG.md) - Detailed version history  
- [CONTRIBUTING.md](CONTRIBUTING.md) - Development guidelines
- [INSTALL.md](INSTALL.md) - Detailed installation instructions

## 🧪 Verified Functionality

This release has been tested and verified for:
- ✅ Core algebraic operations (concatenation, repetition, choice)
- ✅ Porter2 stemming with both lc_alpha and string inputs
- ✅ Mathematical property preservation across operations
- ✅ Memory management and resource cleanup
- ✅ Cross-platform compatibility (Linux, macOS, Windows)

## 🚧 Known Limitations

- Some advanced composition examples may have compilation issues with older compilers
- Full test suite requires C++20 features for complete coverage
- Documentation could benefit from more advanced usage examples

## 🎯 Next Steps

Consider this v1.0.0 as a solid foundation for mathematical text processing. Future versions will focus on:
- Enhanced parser combinator library  
- Unicode support for international text
- Python bindings for research workflows
- Performance optimizations based on real-world usage

---

**Mathematical parsing meets practical performance.** Transform text manipulation from imperative string processing into declarative mathematical expressions.
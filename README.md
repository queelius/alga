# Algebraic Parsers

A mathematically-grounded C++ library for composing and decomposing parsers using algebraic structures. This library applies abstract algebra principles—specifically monoids and functors—to create robust, efficient, and composable parsing solutions.

## Key Features

- **🧮 Mathematical Foundation**: Built on algebraic structures (monoids, functors) for provably correct parser composition
- **⚡ High Performance**: Header-only template-based design with C++17/20 support for zero-cost abstractions
- **🔧 Type Safety**: Leverages the C++ type system to catch errors at compile time
- **🎯 Composability**: Easily combine simple parsers into complex parsing pipelines
- **🧪 Thoroughly Tested**: Comprehensive test suite with >90% coverage and property-based testing
- **📚 Practical Components**: Includes Porter2 stemmer, FSM string rewriters, and combinatorial parsers

## Quick Start

### Prerequisites
- C++17 or later
- CMake 3.14+
- A compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Installation

```bash
git clone https://github.com/yourusername/algebraic_parsers.git
cd algebraic_parsers
mkdir build && cd build
cmake ..
make
```

### Basic Example

```cpp
#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include <iostream>

using namespace algebraic_parsers;

int main() {
    // Create a lower-case alpha string (monoid structure)
    auto word1 = make_lc_alpha("running");
    auto word2 = make_lc_alpha("tests");
    
    if (word1 && word2) {
        // Monoid operation: concatenation
        auto combined = (*word1) * (*word2);
        std::cout << "Combined: " << std::string{combined} << std::endl;
        
        // Porter2 stemming
        porter2_stemmer stemmer;
        auto stem = stemmer(*word1);
        std::cout << "Stem: " << std::string{stem} << std::endl;
    }
    
    return 0;
}
```

## Core Concepts

### 1. Algebraic Structures

The library is built around mathematical abstractions:

- **lc_alpha**: The set of lowercase alphabetic strings forms a free monoid under concatenation
- **porter2_stem**: Stemmed words maintain algebraic properties for composition
- **Functors**: Map parsers while preserving structure

### 2. Parser Combinators

Compose simple parsers into complex ones:

```cpp
#include "parsers/combinatorial_parser.hpp"

// Create parsers for different token types
auto alpha_parser = token_parser<AlphaPredicate, string>{};
auto digit_parser = token_parser<DigitPredicate, int>{};

// Combine them into alternatives
auto mixed_parser = alternative(alpha_parser, digit_parser);
```

### 3. String Processing Pipeline

Build sophisticated text processing workflows:

```cpp
#include "examples/composition_examples.hpp"

using namespace algebraic_parsers::examples;

WordProcessingPipeline pipeline;
auto result = pipeline.process_text("Running tests efficiently!");
// Result: stems and normalized tokens
```

## Library Components

### Core Parsers
- **`lc_alpha`**: Lowercase alphabetic strings with monoid structure
- **`porter2_stemmer`**: Industry-standard Porter2 stemming algorithm  
- **`fsm_string_rewriter`**: Finite state machine-based string transformations
- **`combinatorial_parser`**: Composable parser combinators

### Utility Components
- **`word_parser`**: Configurable word extraction and processing
- **`ngram_stemmer`**: N-gram based stemming with algebraic lifting
- **Composition Examples**: Real-world usage patterns and pipelines

## Advanced Usage

### Custom Parser Creation

```cpp
// Define a custom predicate
struct VowelPredicate {
    bool operator()(char c) const {
        return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
    }
};

// Create a parser for vowels
auto vowel_parser = token_parser<VowelPredicate, char>{};
```

### Error Handling and Recovery

The library provides robust error handling through optional types and monadic composition:

```cpp
auto safe_parse = [](const string& input) -> optional<string> {
    auto lc_word = make_lc_alpha(input);
    if (!lc_word) return nullopt;
    
    porter2_stemmer stemmer;
    auto result = stemmer(*lc_word);
    return string{result};
};
```

## Mathematical Foundations

This library implements several key algebraic concepts:

1. **Free Monoids**: The set of lowercase alphabetic strings with concatenation
2. **Functors**: Structure-preserving mappings between categories of parsers
3. **Composition Laws**: Associativity and identity properties ensure predictable behavior
4. **Type Safety**: Algebraic laws enforced at compile-time through the type system

## Testing

Run the comprehensive test suite:

```bash
cd build
make test
```

For coverage analysis:
```bash
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make coverage
```

## Performance

The library is designed for high performance:
- Header-only templates enable full optimization
- Zero-cost abstractions through modern C++ features
- Efficient algorithms with mathematical guarantees
- Benchmark suite included for performance validation

## Documentation

- [Implementation Guide](IMPLEMENTATION_GUIDE.md)
- [Design Analysis](DESIGN_ANALYSIS.md) 
- [Concrete Examples](CONCRETE_COMPOSITION_EXAMPLES.md)
- [API Reference](docs/) (generated with Doxygen)

## Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

## License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by abstract algebra and category theory
- Built with modern C++ best practices
- Influenced by functional programming parser combinator libraries

---

**Note**: This library brings mathematical rigor to parsing through algebraic structures, making it ideal for applications requiring both performance and correctness guarantees.

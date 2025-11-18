# Alga

A mathematically elegant C++20 library for algebraic text processing and compositional parsing with **fuzzy matching**. Built on rigorous algebraic foundations with monoids, functors, and extended operators, Alga transforms text manipulation from imperative string processing into declarative mathematical expressions.

## ✨ Key Features

- **Mathematical Foundation**: Built on rigorous algebraic structures (monoids, functors) with verified laws and properties
- **Fuzzy Parsing**: Phonetic matching (Soundex, Metaphone), edit distance, and similarity-based parsing for noisy input
- **Unicode Support**: Full UTF-8 encoding/decoding with multi-script alphabetic parsing (Latin, Greek, Cyrillic, Hebrew, Arabic, CJK)
- **Numeric Types**: Type-safe parsers for integers, floats, and scientific notation with algebraic operations
- **Rich Error Reporting**: Position tracking, context extraction, and detailed error diagnostics
- **High Performance**: Header-only template library with zero-cost abstractions and C++20 concepts
- **Type Safety**: Compile-time guarantees through concepts, strong typing, and mathematical invariants
- **Algebraic Operators**: Extended operator set (`*`, `|`, `^`, `%`, `>>`) for mathematical composition
- **Streaming Support**: Memory-efficient parsing of large files with buffered I/O
- **Statistical Analysis**: Frequency counting, entropy, diversity metrics for text analysis
- **Comprehensive Testing**: **425 tests, 100% passing** with property-based testing of algebraic laws
- **Production Ready**: Includes industry-standard Porter2 stemmer and real-world examples

## 📦 Quick Start

### Prerequisites
- **C++20** compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.14+
- Optional: Google Test for running tests

### Installation

```bash
git clone https://github.com/yourusername/alga.git
cd alga
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run tests to verify installation
make test
```

### Basic Example

```cpp
#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
#include "parsers/algebraic_operators.hpp"

using namespace alga;

int main() {
    // Algebraic text processing with mathematical elegance
    auto word1 = make_lc_alpha("hello");
    auto word2 = make_lc_alpha("world");

    if (word1 && word2) {
        // Mathematical composition operators
        auto combined = *word1 * *word2;          // Monoid concatenation: "helloworld"
        auto choice = word1 | word2;              // Choice operator: first valid
        auto emphasis = *word1 ^ 3;               // Repetition: "hellohellohello"
        auto sequence = *word1 >> *word2;         // Sequential: vector["hello", "world"]

        // Porter2 stemming with algebraic composition
        auto stem = make_porter2_stem("running");
        if (stem) {
            auto repeated = *stem ^ 2;             // "runrun"
            std::cout << "Stem: " << std::string{*stem} << std::endl;
        }
    }

    return 0;
}
```

### Fuzzy Parsing Example

```cpp
#include "parsers/fuzzy_parsers.hpp"
#include "parsers/similarity.hpp"

using namespace alga::fuzzy;
using namespace alga::similarity;

int main() {
    // Accept "hello" with up to 2 typos
    auto greeting = fuzzy_match("hello", 2);
    greeting.parse("helo");    // ✓ Matches (1 edit)
    greeting.parse("heello");  // ✓ Matches (1 edit)
    greeting.parse("world");   // ✗ Fails (too different)

    // Sound-alike name matching
    auto name_parser = phonetic_match("Smith");
    name_parser.parse("Smyth");  // ✓ Matches (same Soundex)
    name_parser.parse("Jones");  // ✗ Fails (different sound)

    // Combined fuzzy: case + phonetic + edit distance
    auto flexible = combined_fuzzy("Python", 2);
    flexible.parse("python");  // ✓ Case-insensitive
    flexible.parse("Pyton");   // ✓ Fuzzy match (1 typo)

    // String similarity metrics
    auto dist = levenshtein_distance("kitten", "sitting");  // → 3
    auto sim = jaro_winkler_similarity("Martha", "Marhta"); // → 0.96

    return 0;
}
```

## 🎯 Feature Overview

### Tier 1: Core Parsing Features (185 tests)

#### 1. **Numeric Parser Family** (59 tests)
Type-safe numeric types with algebraic operations:
- `unsigned_int`: Non-negative integers with overflow saturation
- `signed_int`: Signed integers with +/- prefix support
- `floating_point`: Decimal numbers with epsilon comparison
- `scientific_notation`: Exponential format (e.g., "1.5e10")

```cpp
#include "parsers/numeric_parsers.hpp"

auto num = make_unsigned_int("42");
auto sci = make_scientific_notation("1.5e10");
auto sum = *num * *make_unsigned_int("10");  // Monoid addition
```

#### 2. **List Combinators** (26 tests)
Parse separated lists and sequences:
- `sepBy`: Zero or more elements separated by delimiter
- `sepBy1`: One or more (fails on empty)
- `sepEndBy`: Optional trailing separator
- `endBy`: Separator required after each element

```cpp
#include "parsers/list_combinators.hpp"

// CSV parsing
auto csv = sepBy(int_parser(), char_parser(','));
auto [pos, nums] = csv.parse("1,2,3");  // vector<int>{1,2,3}
```

#### 3. **Error Reporting** (48 tests)
Rich error diagnostics with position tracking:
- Line/column/offset position tracking
- Expected vs. found reporting
- Context extraction with visual markers
- `ParseResult<T>` for detailed error info

```cpp
#include "parsers/parse_error.hpp"

auto err = ParseError(position, "unexpected character")
    .expect("digit")
    .expect("letter")
    .but_found("!")
    .with_context(tracker.get_context(20, 20));

std::cout << err.format();
// error at line 5, column 12: unexpected character
//   expected: digit, letter
//   found: !
```

#### 4. **Unicode Support** (52 tests)
UTF-8 aware text processing:
- Full UTF-8 encoding/decoding
- Multi-script alphabetic support (Latin, Greek, Cyrillic, Hebrew, Arabic, CJK)
- Character counting vs. byte counting
- Lowercase conversion for international text

```cpp
#include "parsers/utf8_alpha.hpp"

auto french = make_utf8_alpha("Café");       // → "café"
auto greek = make_utf8_alpha("Αλφα");        // Greek
auto japanese = make_utf8_alpha("こんにちは"); // Hiragana

auto combined = *french * *greek;  // Concatenation works
size_t chars = french->char_count();  // 4 characters, 5 bytes
```

### Tier 2: Convenience Features (119 tests)

#### 1. **Optional Combinators** (25 tests)
Always-succeeding parsers:
- `optional`: Makes parser always succeed (returns nullopt on failure)
- `many`: Zero or more repetitions
- `many1`: One or more (fails on zero)
- `skip`: Parse but discard result

```cpp
#include "parsers/optional_combinator.hpp"

auto opt_sign = optional(char_parser('+'));
auto digits = many1(digit_parser());
```

#### 2. **Count & Range Combinators** (35 tests)
Precise repetition control:
- `count(n, p)`: Exactly n occurrences
- `between(m, n, p)`: Between m and n occurrences
- `atLeast(m, p)`: At least m occurrences
- `atMost(n, p)`: At most n occurrences

```cpp
#include "parsers/count_combinators.hpp"

auto three_digits = count(3, digit_parser());
auto two_to_four = between(2, 4, letter_parser());
```

#### 3. **Streaming Parser** (26 tests)
Memory-efficient large file processing:
- `BufferedStreamReader`: Buffered I/O with look-ahead
- `by_line()`: Line-by-line parsing
- `by_chunks()`: Fixed-size chunk parsing
- `from_file()`: Convenient file parsing

```cpp
#include "parsers/streaming_parser.hpp"

auto parser = from_file("data.txt", word_parser());
parser.parse_by_line([](size_t line_num, const std::string& line, auto result) {
    if (result) {
        process(*result);
    }
});
```

#### 4. **Statistics Module** (33 tests)
Text analysis and metrics:
- Frequency counting with `FrequencyCounter<T>`
- Shannon entropy, normalized entropy
- Simpson diversity, Gini coefficient
- Type-token ratio, hapax/dis legomena
- Comprehensive distribution analysis

```cpp
#include "parsers/statistics.hpp"

FrequencyCounter<std::string> counter;
counter.add_all(words);

double entropy = shannon_entropy(counter);
double diversity = simpson_diversity(counter);
auto top10 = counter.top_n(10);
```

### Tier 3: Fuzzy Parsing (121 tests) 🆕

#### 1. **Phonetic Algorithms** (27 tests)
Sound-alike word matching:
- Soundex encoding (classic algorithm)
- Metaphone encoding (more accurate)
- Sound-alike comparison

```cpp
#include "parsers/phonetic.hpp"

auto code1 = soundex("Smith");   // → "S530"
auto code2 = soundex("Smyth");   // → "S530" (same!)

bool alike = sounds_like_soundex("Robert", "Rupert");  // true
```

#### 2. **Similarity Metrics** (39 tests)
String distance and similarity:
- **Levenshtein distance**: Edit distance (insertions, deletions, substitutions)
- **Damerau-Levenshtein**: Includes transpositions (better for typos)
- **Hamming distance**: Position-by-position differences
- **Jaro similarity**: Good for short strings
- **Jaro-Winkler**: Favors common prefixes
- **LCS**: Longest Common Subsequence

```cpp
#include "parsers/similarity.hpp"

auto dist = levenshtein_distance("kitten", "sitting");  // → 3
auto sim = jaro_winkler_similarity("Martha", "Marhta"); // → 0.96

bool similar = are_similar("hello", "hallo", 0.8);
bool close = within_distance("test", "tset", 2);
```

#### 3. **Fuzzy Parser Combinators** (36 tests)
Integrate fuzzy matching into grammars:
- `fuzzy_match(target, max_dist)`: Accept words within edit distance
- `phonetic_match(target)`: Accept sound-alike words
- `similarity_match(target, threshold)`: Accept similar words
- `fuzzy_choice(candidates, max_dist)`: Match closest candidate
- `case_insensitive(target)`: Ignore case
- `combined_fuzzy(target, max_dist)`: All strategies combined

```cpp
#include "parsers/fuzzy_parsers.hpp"

// Tolerant CSV header parser
auto name_col = fuzzy_match("name", 2);
auto email_col = fuzzy_match("email", 2);
auto header = name_col >> char_parser(',') >> email_col;
header.parse("nme,emai");  // ✓ Accepts typos!

// Multi-strategy command parser
auto exit_cmd = combined_fuzzy("exit", 2);
exit_cmd.parse("Exit");    // ✓ Case-insensitive
exit_cmd.parse("exti");    // ✓ Fuzzy match
exit_cmd.parse("exyt");    // ✓ Phonetic match
```

#### 4. **Text Normalization** (19 tests)
Text cleaning and standardization:
- Case conversion: `to_lowercase()`, `to_uppercase()`, `to_title_case()`
- Whitespace: `trim()`, `normalize_whitespace()`, `remove_whitespace()`
- Filtering: `keep_alpha()`, `keep_alnum()`, `remove_punctuation()`
- Transformation: `to_slug()`, `replace_all()`, `collapse_repeated()`
- Comprehensive: `normalize_text()` (lowercase + whitespace normalization)

```cpp
#include "parsers/normalization.hpp"

auto clean = normalize_text("  HELLO   WORLD  ");  // → "hello world"
auto slug = to_slug("Hello World!");                // → "hello-world"
auto title = to_title_case("hello world");          // → "Hello World"
```

## 🏗️ Architecture

```
include/parsers/
├── Core Types (Tier 0)
│   ├── lc_alpha.hpp              # Free monoid of lowercase strings
│   ├── porter2stemmer.hpp         # Porter2 stemming algorithm
│   ├── ngram_stemmer.hpp          # N-gram generation
│   ├── word_parser.hpp            # Word tokenization
│   └── fsm_string_rewriter.hpp    # FSM-based transformations
│
├── Tier 1: Core Parsing Features
│   ├── numeric_parsers.hpp        # unsigned_int, signed_int, floating_point, scientific_notation
│   ├── list_combinators.hpp       # sepBy, sepBy1, sepEndBy, endBy
│   ├── parse_error.hpp            # Rich error reporting with position tracking
│   └── utf8_alpha.hpp             # UTF-8 text processing, multi-script support
│
├── Tier 2: Convenience Features
│   ├── optional_combinator.hpp    # optional, many, many1, skip
│   ├── count_combinators.hpp      # count, between, atLeast, atMost
│   ├── streaming_parser.hpp       # BufferedStreamReader, by_line, by_chunks
│   └── statistics.hpp             # FrequencyCounter, entropy, diversity metrics
│
├── Tier 3: Fuzzy Parsing
│   ├── phonetic.hpp               # Soundex, Metaphone encoding
│   ├── similarity.hpp             # Levenshtein, Jaro-Winkler, LCS
│   ├── fuzzy_parsers.hpp          # fuzzy_match, phonetic_match, combined_fuzzy
│   └── normalization.hpp          # Text cleaning and standardization
│
└── Algebraic Operations
    ├── algebraic_operators.hpp    # |, ^, %, >> operators
    ├── combinatorial_parser_fixed.hpp  # Parser combinator framework
    └── monadic_combinators.hpp    # >>=, fmap

test/                              # Comprehensive test suite (425 tests)
├── Tier 1 Tests (185 tests)
│   ├── numeric_parsers_test.cpp   # 59 tests
│   ├── list_combinators_test.cpp  # 26 tests
│   ├── parse_error_test.cpp       # 48 tests
│   └── utf8_alpha_test.cpp        # 52 tests
│
├── Tier 2 Tests (119 tests)
│   ├── optional_combinator_test.cpp  # 25 tests
│   ├── count_combinators_test.cpp    # 35 tests
│   ├── streaming_parser_test.cpp     # 26 tests
│   └── statistics_test.cpp           # 33 tests
│
└── Tier 3 Tests (121 tests)
    ├── phonetic_test.cpp          # 27 tests
    ├── similarity_test.cpp        # 39 tests
    ├── fuzzy_parsers_test.cpp     # 36 tests
    └── normalization_test.cpp     # 19 tests
```

## 🔬 Mathematical Foundations

### Algebraic Structures

The library implements several key mathematical concepts:

1. **Free Monoids**: The set of lowercase alphabetic strings forms a free monoid under concatenation
   - **Identity**: Empty string
   - **Associativity**: (a · b) · c = a · (b · c)
   - **Closure**: Concatenation always produces valid strings

2. **Functors**: Structure-preserving transformations between categories
   ```cpp
   // fmap preserves composition: fmap(g ∘ f) = fmap(g) ∘ fmap(f)
   auto parser = make_word_parser();
   auto length_parser = fmap(parser, [](const std::string& s) { return s.length(); });
   ```

3. **Monadic Composition**: Error-handling composition with automatic failure propagation
   ```cpp
   auto result = make_lc_alpha("running")
       >>= [](const lc_alpha& word) { return stemmer(word); }
       >>= [](const porter2_stem& stem) { return make_unigram(stem); };
   ```

### Verified Properties

All algebraic structures satisfy their mathematical laws:

```cpp
// Monoid laws (automatically tested)
ASSERT_EQ((a * b) * c, a * (b * c));           // Associativity
ASSERT_EQ(empty * a, a);                        // Left identity
ASSERT_EQ(a * empty, a);                        // Right identity

// Functor laws (automatically tested)
ASSERT_EQ(fmap(id, parser), parser);            // Identity preservation
ASSERT_EQ(fmap(compose(g,f), parser), fmap(g, fmap(f, parser))); // Composition
```

## 📊 Testing and Quality

### Comprehensive Test Suite

The library includes **425 tests** across all tiers:

```bash
# Run all tests
make test

# Individual test suites
./test/numeric_parsers_test      # 59 tests
./test/list_combinators_test     # 26 tests
./test/parse_error_test          # 48 tests
./test/utf8_alpha_test           # 52 tests
./test/optional_combinator_test  # 25 tests
./test/count_combinators_test    # 35 tests
./test/streaming_parser_test     # 26 tests
./test/statistics_test           # 33 tests
./test/phonetic_test             # 27 tests
./test/similarity_test           # 39 tests
./test/fuzzy_parsers_test        # 36 tests
./test/normalization_test        # 19 tests

# Test coverage analysis
make coverage  # Requires CMAKE_BUILD_TYPE=Coverage
```

### Property-Based Testing

Mathematical properties are automatically verified:

```cpp
// Test algebraic laws for all monoid types
TEST_F(AlgebraicProperties, MonoidLaws) {
    auto a = *make_lc_alpha("hello");
    auto b = *make_lc_alpha("beautiful");
    auto c = *make_lc_alpha("world");

    EXPECT_EQ((a * b) * c, a * (b * c));  // Associativity
    EXPECT_EQ(lc_alpha{} * a, a);         // Left identity
    EXPECT_EQ(a * lc_alpha{}, a);         // Right identity
}
```

## ⚡ Performance

- **Zero-Cost Abstractions**: Template metaprogramming eliminates runtime overhead
- **Memory Efficient**: RAII and move semantics, no unnecessary copying
- **Parallelizable**: Value semantics enable safe concurrent processing
- **Optimized Algorithms**: Tuned implementations of classic algorithms
- **Streaming Support**: Process large files without loading into memory

Benchmarks show comparable performance to hand-optimized C code while providing mathematical guarantees and composability.

## 🤝 Contributing

We welcome contributions that maintain the mathematical rigor and performance characteristics:

1. **Mathematical Correctness**: All algebraic structures must satisfy their laws
2. **Type Safety**: Use C++20 concepts and static assertions to prevent invalid usage
3. **Performance**: Maintain zero-cost abstraction principles
4. **Testing**: Include comprehensive tests with property-based verification
5. **Documentation**: Provide clear mathematical explanations

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## 📄 License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by Haskell's parser combinator libraries and category theory
- Built using modern C++20 best practices and mathematical foundations
- Implements classical algorithms (Porter2, Soundex) within a principled algebraic framework
- Fuzzy parsing techniques adapted from information retrieval research

---

**Mathematical parsing meets practical performance.** This library demonstrates that rigorous mathematical foundations enhance rather than compromise real-world usability and performance—now with fuzzy matching for noisy real-world data!

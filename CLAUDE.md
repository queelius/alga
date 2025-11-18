# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Alga is a mathematically rigorous C++20 header-only template library implementing algebraic parsers with monadic composition. The library models text processing as composable algebraic structures (monoids, functors) with type safety and zero-cost abstractions.

## Build System

### CMake Workflow (Primary)

```bash
# Standard build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run all tests via CTest
make test

# Individual test suites
./composition_tests
./unit_tests
./integration_tests

# Build specific components
make lc_alpha_test
make porter2stemmer_test
```

### Manual Compilation (Direct g++)

```bash
# Compile any test file with GoogleTest
g++ -std=c++20 -I. -Iinclude test/<test_file>.cpp include/parsers/porter2stemmer.cpp \
    -lgtest -lgtest_main -pthread -o test/<test_name>

# Compile standalone test without GoogleTest
g++ -std=c++20 -I. -Iinclude test/<test_file>.cpp include/parsers/porter2stemmer.cpp \
    -o test/<test_name>

# Run with coverage
g++ -std=c++20 -I. -Iinclude --coverage test/<test_file>.cpp \
    include/parsers/porter2stemmer.cpp -lgtest -lgtest_main -pthread \
    -o test/<test_name>_cov
./test/<test_name>_cov
gcov test/<test_file>.cpp
```

### Coverage Analysis

```bash
# CMake coverage build
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make coverage  # Generates HTML report in coverage_html/

# Manual gcov workflow
g++ -std=c++20 --coverage -I. -Iinclude test/file.cpp include/parsers/porter2stemmer.cpp \
    -lgtest -lgtest_main -pthread -o test/file_cov
./test/file_cov
gcov test/file.cpp
```

### Development Targets

```bash
make format            # Code formatting (requires clang-format)
make static_analysis   # Static analysis (requires cppcheck)
make docs             # Generate Doxygen documentation
```

## Architecture

### Namespace Structure

**Primary namespace**: `alga` (transitioned from `algebraic_parsers` in v1.0.0)

- `alga::`: Core parser types and factory functions
- `alga::combinatorial::`: Parser combinator framework
- `alga::operators::`: Extended algebraic operators (`|`, `^`, `%`, `>>`)
- `alga::concepts::`: C++20 concepts for type constraints

### Core Design Principles

1. **Pure Optional Pattern**: All fallible operations return `std::optional<T>`
2. **Value Semantics**: All types are copyable, movable, and container-compatible (no const members)
3. **Factory Functions**: `make_<type>()` factories are the only safe constructors from external input
4. **Algebraic Consistency**: Uniform operator set across all types: `*` (composition), `==`, `!=`, `<`, `<<`
5. **Monadic Composition**: Error propagation through optional chaining and monadic bind (`>>=`)

### Key Components

**Core Parser Types** (in `include/parsers/`):

- **lc_alpha.hpp**: Type-safe lowercase alphabetic strings forming a free monoid
  - Factory: `make_lc_alpha(string_view) -> optional<lc_alpha>`
  - Validates input, converts to lowercase, rejects non-alphabetic characters
  - Monoid: `operator*` for concatenation, empty string as identity

- **porter2stemmer.hpp**: Porter2 stemming algorithm with algebraic interface
  - Factory: `make_porter2_stem(string_view) -> optional<porter2_stem>`
  - Function: `stemmer(lc_alpha) -> optional<porter2_stem>`
  - Wraps `lc_alpha` with stemming transformation

- **ngram_stemmer.hpp**: N-gram generation and composition
  - Factories: `make_unigram()`, `make_bigram()`, etc.
  - Template-based: `ngram_stem<N, T>`

- **algebraic_operators.hpp**: Extended operator implementations using C++20 concepts
  - `|` (choice): first valid alternative
  - `^` (repetition): compose element N times
  - `%` (map): apply function to value
  - `>>` (sequence): compose into vector

- **combinatorial_parser_fixed.hpp**: Generic parser combinators
- **monadic_combinators.hpp**: Monadic operations (`>>=`, `fmap`)
- **word_parser.hpp**: Word tokenization
- **fsm_string_rewriter.hpp**: Finite state machine-based transformations

### NEW: Numeric Parser Family (Tier 1)

**numeric_parsers.hpp**: Type-safe numeric types with algebraic operations

- **unsigned_int**: Non-negative integers with overflow saturation
  - Factory: `make_unsigned_int(string_view) -> optional<unsigned_int>`
  - Factory: `make_unsigned_int(uint64_t) -> optional<unsigned_int>`
  - Monoid operation: addition (with overflow detection)
  - Range: 0 to 2^64-1

- **signed_int**: Signed integers with +/- prefix support
  - Factory: `make_signed_int(string_view) -> optional<signed_int>`
  - Accepts: "+123", "-456", "789"
  - Monoid operation: addition (with overflow saturation)

- **floating_point**: Decimal numbers with epsilon comparison
  - Factory: `make_floating_point(string_view) -> optional<floating_point>`
  - Accepts: "3.14", "-2.5", ".5", "42"
  - Epsilon-based equality (1e-10)

- **scientific_notation**: Exponential format numbers
  - Factory: `make_scientific_notation(string_view) -> optional<scientific_notation>`
  - Accepts: "1.5e10", "3.2E-5", "5e3"
  - Must contain 'e' or 'E'

All numeric types support:
- Full value semantics (copy, move, container storage)
- Algebraic operators: `*`, `+`, `|`, `^`, `%`
- Comparison operators with appropriate semantics
- Stream output

### NEW: List Combinators (Tier 1)

**list_combinators.hpp**: Parse separated lists and sequences

```cpp
namespace alga::combinators {
  // Zero or more elements separated by delimiter
  auto sepBy(parser, separator) -> SepByParser

  // One or more elements (fails on empty)
  auto sepBy1(parser, separator) -> SepBy1Parser

  // Optional trailing separator allowed
  auto sepEndBy(parser, separator) -> SepEndByParser

  // Separator required after each element
  auto endBy(parser, separator) -> EndByParser

  // Helper parsers
  auto char_parser(char) -> CharParser
  auto whitespace() -> WhitespaceParser
  auto optional_whitespace() -> OptionalWhitespaceParser
}
```

**Common use cases**:
```cpp
// CSV parsing
auto csv = sepBy(int_parser(), char_parser(','));
auto [pos, nums] = csv.parse("1,2,3");  // vector<int>{1,2,3}

// Config files
auto config = endBy(word_parser(), char_parser(';'));
auto [pos, words] = config.parse("key;value;port;");

// Whitespace-separated
auto tokens = sepBy(word_parser(), whitespace());
```

### NEW: Error Reporting System (Tier 1)

**parse_error.hpp**: Rich error diagnostics with position tracking

**Position tracking**:
```cpp
namespace alga::error {
  // Line/column/offset position (1-indexed)
  struct Position {
    size_t line, column, offset;
    void advance(char c);  // Track newlines automatically
  };

  // Source text span
  struct Span {
    Position start, end;
    size_t length() const;
  };

  // Position tracker for iterators
  template<typename Iterator>
  class PositionTracker {
    Position position() const;
    void advance();
    std::optional<char> peek() const;
    std::string get_context(size_t before, size_t after) const;
    Span span_from(Position start) const;
  };
}
```

**Error building**:
```cpp
// Rich error construction with builder pattern
auto err = ParseError(position, "unexpected character")
    .expect("digit")
    .expect("letter")
    .but_found("!")
    .with_context(tracker.get_context(20, 20));

// Formatted output
std::cout << err.format();
// error at line 5, column 12: unexpected character
//   expected: digit, letter
//   found: !
//   context: ...quick⮜brown fox...
```

**ParseResult alternative to optional**:
```cpp
// Either success with value OR detailed error
ParseResult<int> result = parse_int(input);
if (result.success()) {
    int value = result.value();
} else {
    std::cout << result.error().format();
}

// Convert to optional (loses error info)
std::optional<int> opt = result.to_optional();
```

**Common error builders**:
- `errors::expected_char(pos, expected, found)`
- `errors::expected_one_of(pos, chars, found)`
- `errors::expected_eof(pos, found)`
- `errors::unexpected_eof(pos, expected)`
- `errors::invalid_format(pos, what, details)`
- `errors::custom(pos, message)`

### NEW: Unicode Support (Tier 1)

**utf8_alpha.hpp**: UTF-8 aware text processing

**UTF-8 utilities**:
```cpp
namespace alga::utf8 {
  // Low-level UTF-8 operations
  size_t utf8_sequence_length(unsigned char first_byte);
  bool is_valid_utf8_sequence(string_view);
  bool is_valid_utf8(string_view);

  // Encode/decode Unicode code points
  optional<uint32_t> decode_utf8(string_view);
  optional<string> encode_utf8(uint32_t codepoint);

  // Character classification
  bool is_unicode_alpha(uint32_t codepoint);
  uint32_t to_lowercase(uint32_t codepoint);
}
```

**Supported Unicode scripts**:
- Latin (including Extended-A, Extended-B)
- Greek and Coptic
- Cyrillic
- Hebrew
- Arabic
- Hiragana/Katakana
- CJK Unified Ideographs

**utf8_alpha type**:
```cpp
// UTF-8 aware alphabetic strings
auto french = make_utf8_alpha("Café");       // → "café" (lowercase)
auto greek = make_utf8_alpha("Αλφα");        // Greek letters
auto russian = make_utf8_alpha("Привет");    // Cyrillic
auto japanese = make_utf8_alpha("こんにちは"); // Hiragana

// All algebraic operations work
auto combined = *french * *greek;  // Concatenation
auto repeated = *french ^ 3;       // Repetition

// Character counting (not byte counting)
size_t chars = french->char_count();  // 4 characters
size_t bytes = french->size();        // 5 bytes (é is 2 bytes)

// Get Unicode code points
std::vector<uint32_t> cps = french->codepoints();
```

**Validation**:
- Validates UTF-8 encoding
- Accepts only alphabetic characters
- Converts to lowercase
- Rejects invalid UTF-8 sequences
- Rejects mixed alphabetic/numeric input

### Key Abstractions

**Value Semantics Design**:
```cpp
// All types support full value semantics
lc_alpha a = *make_lc_alpha("hello");
lc_alpha b = a;              // Copy
lc_alpha c = std::move(a);   // Move
std::vector<lc_alpha> words; // Container storage
words.push_back(b);          // No const member issues
```

**Uniform Optional Pattern**:
```cpp
// All factories return optional<T>
auto word = make_lc_alpha("hello");      // optional<lc_alpha>
if (!word) { /* handle error */ }

auto stem = make_porter2_stem("running"); // optional<porter2_stem>
auto ngram = make_unigram(*stem);         // optional<unigram_stem>
```

**Algebraic Composition**:
```cpp
// Monoid concatenation (*)
auto a = *make_lc_alpha("hello");
auto b = *make_lc_alpha("world");
auto c = a * b;  // "helloworld"

// Choice operator (|)
auto choice = word1 | word2;  // First valid value

// Repetition operator (^)
auto repeated = a ^ 3;  // "hellohellohello"

// Functional map (%)
auto length = word % [](auto w) { return w.size(); };
```

## Testing Structure

**Core Test Suites**:
- `unit_tests.cpp`: Individual component unit tests with GTest
- `composition_test.cpp`: Algebraic composition and property tests
- `integration_tests.cpp`: End-to-end document processing tests
- `comprehensive_alga_tests.cpp`: Core alga namespace functionality
- `core_alga_tests.cpp`: Fundamental algebraic structure tests
- `edge_case_tests.cpp`: Boundary conditions and error cases
- `uniform_optional_pattern_tests.cpp`: Optional pattern consistency
- `performance_benchmark.cpp`: Performance validation

**NEW: Tier 1 Feature Tests** (185 tests total):
- `numeric_parsers_test.cpp`: All numeric types (59 tests)
  - unsigned_int, signed_int, floating_point, scientific_notation
  - Algebraic laws, overflow handling, value semantics
- `list_combinators_test.cpp`: List parsing (26 tests)
  - sepBy, sepBy1, sepEndBy, endBy combinators
  - Integration with algebraic types
- `parse_error_test.cpp`: Error reporting (48 tests)
  - Position tracking, error building, ParseResult
  - Context extraction, formatting
- `utf8_alpha_test.cpp`: Unicode support (52 tests)
  - UTF-8 encoding/decoding, validation
  - Multi-script support (Latin, Greek, Cyrillic, Hebrew, Arabic, CJK)

**Running Tier 1 Tests**:
```bash
# Individual test suites
./test/numeric_parsers_test     # 59 tests
./test/list_combinators_test    # 26 tests
./test/parse_error_test         # 48 tests
./test/utf8_alpha_test          # 52 tests

# Or compile and run
g++ -std=c++20 -I. -Iinclude test/numeric_parsers_test.cpp -lgtest -lgtest_main -pthread -o test/numeric_parsers_test && ./test/numeric_parsers_test
```

**Test Coverage**: Target 90%+ with property-based testing of algebraic laws (associativity, identity, etc.)

## Important Constraints

- **C++20 Required**: Library now requires `-std=c++20` for concepts and advanced template features
- **Header-Only**: Most components are header-only except `porter2stemmer.cpp`
- **No Mutable State**: All operations are pure with value semantics
- **Exception Safety**: Uses optional for error handling, no exceptions from library code
- **Zero Dependencies**: Only standard library (except tests use GoogleTest)

## Common Patterns

### Creating and Validating Input
```cpp
// Always use factory functions
auto word = make_lc_alpha("input");
if (!word) {
    // Handle validation failure
    return;
}

// Chaining operations
auto result = make_lc_alpha("running")
    .and_then([](auto w) { return stemmer(w); })
    .and_then([](auto s) { return make_unigram(s); });
```

### Testing Algebraic Laws
```cpp
// Monoid associativity
TEST(AlgebraicLaws, Associativity) {
    auto a = *make_lc_alpha("hello");
    auto b = *make_lc_alpha("beautiful");
    auto c = *make_lc_alpha("world");
    EXPECT_EQ((a * b) * c, a * (b * c));
}

// Monoid identity
TEST(AlgebraicLaws, Identity) {
    auto empty = lc_alpha{};
    auto word = *make_lc_alpha("test");
    EXPECT_EQ(empty * word, word);
    EXPECT_EQ(word * empty, word);
}
```

## Version Notes

**v1.0.0 Breaking Changes**:
- Namespace changed from `algebraic_parsers` to `alga`
- All types redesigned with complete value semantics (no const members)
- Uniform optional pattern for all factories
- C++20 required (was C++17)
- Extended operator set with C++20 concepts

## Documentation

- `README.md`: Comprehensive library overview and examples
- `INSTALL.md`: Detailed build and installation instructions
- `CONTRIBUTING.md`: Development guidelines
- `CHANGELOG.md`: Version history and breaking changes
- Doxygen: API reference (generate with `make docs`)

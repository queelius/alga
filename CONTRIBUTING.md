# Contributing to Algebraic Parsers

Thank you for your interest in contributing to the Algebraic Parsers library! This document provides guidelines for contributing code, reporting issues, and helping improve the project.

## Code of Conduct

We are committed to providing a welcoming and inclusive environment for all contributors. Please be respectful and considerate in all interactions.

## How to Contribute

### Reporting Issues

Before creating an issue, please:
1. Search existing issues to avoid duplicates
2. Use the latest version to confirm the issue still exists
3. Provide a minimal reproducible example

When creating an issue, include:
- **Environment**: OS, compiler, C++ standard version
- **Expected behavior**: What should happen
- **Actual behavior**: What actually happens
- **Minimal example**: Code that demonstrates the issue
- **Error messages**: Full compiler/runtime error output

### Suggesting Features

Feature requests are welcome! Please:
1. Check if the feature already exists or is planned
2. Explain the use case and benefits
3. Consider if it fits the library's mathematical/algebraic design philosophy
4. Be willing to help implement or test the feature

### Contributing Code

#### Getting Started

1. Fork the repository
2. Clone your fork:
   ```bash
   git clone https://github.com/yourusername/algebraic_parsers.git
   cd algebraic_parsers
   ```
3. Create a feature branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```

#### Development Setup

1. Install dependencies (see [INSTALL.md](INSTALL.md))
2. Build the project:
   ```bash
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```
3. Run tests to ensure everything works:
   ```bash
   make test
   ```

#### Code Standards

##### C++ Style Guidelines

- **Standard**: Follow C++17/20 best practices
- **Naming**: 
  - Classes: `snake_case` (e.g., `lc_alpha`)
  - Functions: `snake_case` (e.g., `make_lc_alpha`)
  - Variables: `snake_case`
  - Constants: `UPPER_SNAKE_CASE`
- **Headers**: Use `#pragma once`
- **Includes**: Standard library first, then project headers
- **Formatting**: Use clang-format (config provided)

##### Mathematical Rigor

This library emphasizes mathematical correctness:
- **Document algebraic properties** in comments
- **Preserve mathematical invariants** in implementations  
- **Use precise mathematical terminology** in naming and documentation
- **Provide proofs or references** for non-trivial algorithms
- **Test algebraic properties** (associativity, identity, etc.)

##### Example Code Style

```cpp
#pragma once

#include <optional>
#include <string>
#include "lc_alpha.hpp"

namespace algebraic_parsers {
    /**
     * A monoid structure representing stemmed words.
     * 
     * Mathematical properties:
     * - Identity: empty stem
     * - Associativity: (a * b) * c = a * (b * c)
     * - Closure: stem * stem -> stem
     */
    struct porter2_stem {
        using element_type = char;
        using value_type = lc_alpha;

        // Explicit conversion maintains type safety
        explicit operator std::string() const { return std::string{w}; }
        explicit operator lc_alpha() const { return w; }

        // Iterator interface for generic algorithms
        auto begin() const { return w.begin(); }
        auto end() const { return w.end(); }

    private:
        friend optional<porter2_stem> make_porter2_stem(lc_alpha);
        explicit porter2_stem(lc_alpha w) : w(std::move(w)) {}
        
        // Invariant: w is a valid Porter2 stem
        lc_alpha const w;
    };
}
```

#### Testing Requirements

All contributions must include tests:

##### Unit Tests
```cpp
TEST(LcAlphaTest, MonoidIdentity) {
    auto word = make_lc_alpha("test");
    auto identity = lc_alpha{};  // Identity element
    
    ASSERT_TRUE(word.has_value());
    
    // Left identity: e * a = a
    EXPECT_EQ(identity * (*word), *word);
    
    // Right identity: a * e = a  
    EXPECT_EQ((*word) * identity, *word);
}

TEST(LcAlphaTest, MonoidAssociativity) {
    auto a = make_lc_alpha("hello");
    auto b = make_lc_alpha("beautiful");
    auto c = make_lc_alpha("world");
    
    ASSERT_TRUE(a && b && c);
    
    // Associativity: (a * b) * c = a * (b * c)
    auto left = ((*a) * (*b)) * (*c);
    auto right = (*a) * ((*b) * (*c));
    
    EXPECT_EQ(left, right);
}
```

##### Property-Based Tests
```cpp
TEST(PorterStemmerTest, StemIdempotence) {
    porter2_stemmer stemmer;
    
    for (const auto& word : test_words) {
        auto lc_word = make_lc_alpha(word);
        if (!lc_word) continue;
        
        auto stem1 = stemmer(*lc_word);
        auto stem2 = stemmer(lc_alpha{stem1});
        
        // Property: stemming is idempotent
        EXPECT_EQ(stem1, stem2) 
            << "Stemming should be idempotent for word: " << word;
    }
}
```

##### Performance Tests
Include benchmarks for performance-critical code:
```cpp
TEST(PerformanceTest, LcAlphaCreation) {
    constexpr size_t iterations = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < iterations; ++i) {
        volatile auto result = make_lc_alpha("testword");
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should create 1M lc_alpha objects in under 100ms
    EXPECT_LT(duration.count(), 100000);
}
```

#### Documentation Requirements

- **API Documentation**: Use Doxygen comments for all public interfaces
- **Mathematical Documentation**: Explain algebraic properties and invariants
- **Examples**: Provide usage examples for new features
- **Design Rationale**: Explain why design decisions were made

Example documentation:
```cpp
/**
 * @brief Creates a validated lowercase alphabetic string.
 * 
 * This function implements the constructor for the free monoid of lowercase 
 * alphabetic strings. The resulting lc_alpha object satisfies the monoid laws:
 * 
 * 1. **Closure**: For any valid strings s1, s2, lc_alpha(s1) * lc_alpha(s2) 
 *    is also a valid lc_alpha.
 * 2. **Associativity**: (a * b) * c = a * (b * c)
 * 3. **Identity**: empty lc_alpha acts as identity element
 * 
 * @param x Input string to validate and convert
 * @return optional<lc_alpha> containing the validated string, or nullopt if 
 *         the input contains non-alphabetic characters
 * 
 * @complexity O(n) where n is the length of the input string
 * 
 * @example
 * ```cpp
 * auto word = make_lc_alpha("Hello");
 * if (word) {
 *     std::cout << std::string{*word} << std::endl;  // Prints: hello
 * }
 * 
 * auto invalid = make_lc_alpha("Hello123");
 * assert(!invalid);  // Contains non-alpha characters
 * ```
 */
std::optional<lc_alpha> make_lc_alpha(std::string x);
```

#### Pull Request Process

1. **Ensure tests pass**: All existing and new tests must pass
2. **Update documentation**: Include relevant documentation updates
3. **Add changelog entry**: Add entry to CHANGELOG.md (if it exists)
4. **Rebase on main**: Ensure your branch is up-to-date
5. **Create pull request**: Use the provided template

##### Pull Request Template

```markdown
## Description
Brief description of the changes and their purpose.

## Type of Change
- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that causes existing functionality to change)
- [ ] Documentation update
- [ ] Performance improvement
- [ ] Code refactoring

## Mathematical Properties
If applicable, describe the algebraic properties maintained or added:
- [ ] Preserves existing monoid/functor laws
- [ ] Adds new algebraic structures
- [ ] Maintains type safety guarantees

## Testing
- [ ] Unit tests added/updated
- [ ] Property-based tests added for algebraic properties
- [ ] Performance tests added (if applicable)
- [ ] All tests pass locally
- [ ] Test coverage maintained or improved

## Documentation
- [ ] API documentation updated
- [ ] Examples provided
- [ ] Design rationale documented
- [ ] Mathematical properties documented

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex algorithms
- [ ] No unnecessary dependencies added
- [ ] Backwards compatibility preserved (or breaking changes documented)
```

#### Review Process

1. **Automated checks**: CI will run tests, formatting, and static analysis
2. **Peer review**: At least one maintainer will review the code
3. **Mathematical review**: Complex algorithmic changes may require additional mathematical review
4. **Integration testing**: Changes will be tested in combination with existing features

## Development Workflow

### Branch Naming
- Features: `feature/description`
- Bug fixes: `bugfix/issue-number-description`
- Documentation: `docs/description`
- Performance: `perf/description`

### Commit Messages
Follow conventional commit format:
```
type(scope): brief description

Detailed description if necessary.

- Additional bullet points
- Reference issue numbers (#123)
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`

### Examples
```
feat(parser): add support for custom predicates

Implement generic predicate interface for token parsers,
allowing users to define custom character classification
rules while maintaining algebraic properties.

- Add Predicate concept definition
- Implement token_parser template
- Add comprehensive tests for custom predicates
- Update documentation with examples

Fixes #42
```

## Release Process

Releases follow semantic versioning (SemVer):
- **MAJOR**: Breaking changes to public API
- **MINOR**: New features (backwards compatible)  
- **PATCH**: Bug fixes (backwards compatible)

## Getting Help

- **Questions**: Use GitHub Discussions
- **Bugs**: Create GitHub Issues
- **Chat**: Join our development chat (link in README)
- **Email**: Contact maintainers directly for sensitive issues

## Recognition

Contributors will be recognized in:
- CONTRIBUTORS.md file
- Release notes
- Documentation acknowledgments

Thank you for helping make Algebraic Parsers better!
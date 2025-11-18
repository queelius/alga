#pragma once

#include <optional>
#include <vector>
#include <string_view>
#include <type_traits>
#include <utility>

namespace alga {
namespace combinators {

/**
 * @brief sepBy - Parse zero or more occurrences of parser separated by separator
 *
 * Parses: p (sep p)*
 * Example: "1,2,3" with integer parser and ',' separator
 * Returns: vector of parsed elements (empty vector if no elements found)
 *
 * This is a greedy combinator - it consumes as many elements as possible.
 * It always succeeds (returns empty vector if no elements).
 */
template<typename ElementParser, typename SeparatorParser>
class SepByParser {
public:
    ElementParser element_parser;
    SeparatorParser separator_parser;

    SepByParser(ElementParser ep, SeparatorParser sp)
        : element_parser(std::move(ep)), separator_parser(std::move(sp)) {}

    using element_type = typename ElementParser::output_type;
    using output_type = std::vector<element_type>;

    /**
     * @brief Parse separated list from iterator range
     *
     * @return pair<Iterator, optional<vector<T>>> where:
     *         - Iterator points to position after last parsed element
     *         - optional contains the vector of elements (never nullopt for sepBy)
     */
    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        Iterator current = begin;

        // Try to parse first element
        auto [pos1, first_result] = element_parser.parse(current, end);
        if (!first_result) {
            // No elements found - return empty vector (success with 0 elements)
            return {begin, std::make_optional(output_type{})};
        }

        results.push_back(std::move(*first_result));
        current = pos1;

        // Parse remaining elements: (sep element)*
        while (current != end) {
            // Try to parse separator
            auto [sep_pos, sep_result] = separator_parser.parse(current, end);
            if (!sep_result) {
                // No more separators - done
                break;
            }

            // Separator found, now element must follow
            auto [elem_pos, elem_result] = element_parser.parse(sep_pos, end);
            if (!elem_result) {
                // Separator without following element - stop here
                // Don't consume the separator
                break;
            }

            results.push_back(std::move(*elem_result));
            current = elem_pos;
        }

        return {current, std::make_optional(std::move(results))};
    }

    /**
     * @brief Parse from string_view
     */
    auto operator()(std::string_view input) const
        -> std::pair<std::string_view::iterator, std::optional<output_type>>
    {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief sepBy1 - Parse one or more occurrences separated by separator
 *
 * Parses: p (sep p)+
 * Example: "1,2,3" with integer parser and ',' separator
 * Returns: optional<vector> (nullopt if zero elements found)
 *
 * This combinator FAILS if no elements are found (unlike sepBy).
 */
template<typename ElementParser, typename SeparatorParser>
class SepBy1Parser {
public:
    ElementParser element_parser;
    SeparatorParser separator_parser;

    SepBy1Parser(ElementParser ep, SeparatorParser sp)
        : element_parser(std::move(ep)), separator_parser(std::move(sp)) {}

    using element_type = typename ElementParser::output_type;
    using output_type = std::vector<element_type>;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        Iterator current = begin;

        // MUST parse at least one element
        auto [pos1, first_result] = element_parser.parse(current, end);
        if (!first_result) {
            // No elements found - FAILURE
            return {begin, std::nullopt};
        }

        results.push_back(std::move(*first_result));
        current = pos1;

        // Parse remaining elements: (sep element)*
        while (current != end) {
            auto [sep_pos, sep_result] = separator_parser.parse(current, end);
            if (!sep_result) {
                break;
            }

            auto [elem_pos, elem_result] = element_parser.parse(sep_pos, end);
            if (!elem_result) {
                break;
            }

            results.push_back(std::move(*elem_result));
            current = elem_pos;
        }

        return {current, std::make_optional(std::move(results))};
    }

    auto operator()(std::string_view input) const
        -> std::pair<std::string_view::iterator, std::optional<output_type>>
    {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief sepEndBy - Parse zero or more occurrences with optional trailing separator
 *
 * Parses: (p sep)* p?
 * Example: "1,2,3," with integer parser and ',' separator
 * Returns: vector of elements (allows trailing separator)
 */
template<typename ElementParser, typename SeparatorParser>
class SepEndByParser {
public:
    ElementParser element_parser;
    SeparatorParser separator_parser;

    SepEndByParser(ElementParser ep, SeparatorParser sp)
        : element_parser(std::move(ep)), separator_parser(std::move(sp)) {}

    using element_type = typename ElementParser::output_type;
    using output_type = std::vector<element_type>;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        Iterator current = begin;

        while (current != end) {
            // Try to parse element
            auto [elem_pos, elem_result] = element_parser.parse(current, end);
            if (!elem_result) {
                // No more elements
                break;
            }

            results.push_back(std::move(*elem_result));
            current = elem_pos;

            // Try to parse separator (optional)
            auto [sep_pos, sep_result] = separator_parser.parse(current, end);
            if (sep_result) {
                current = sep_pos;
                // Continue to look for more elements
            } else {
                // No separator - done
                break;
            }
        }

        return {current, std::make_optional(std::move(results))};
    }

    auto operator()(std::string_view input) const
        -> std::pair<std::string_view::iterator, std::optional<output_type>>
    {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief endBy - Parse zero or more occurrences terminated by separator
 *
 * Parses: (p sep)*
 * Example: "1;2;3;" with integer parser and ';' separator
 * Returns: vector of elements (separator is REQUIRED after each element)
 */
template<typename ElementParser, typename SeparatorParser>
class EndByParser {
public:
    ElementParser element_parser;
    SeparatorParser separator_parser;

    EndByParser(ElementParser ep, SeparatorParser sp)
        : element_parser(std::move(ep)), separator_parser(std::move(sp)) {}

    using element_type = typename ElementParser::output_type;
    using output_type = std::vector<element_type>;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        Iterator current = begin;

        while (current != end) {
            // Try to parse element
            auto [elem_pos, elem_result] = element_parser.parse(current, end);
            if (!elem_result) {
                // No more elements
                break;
            }

            // Separator MUST follow element
            auto [sep_pos, sep_result] = separator_parser.parse(elem_pos, end);
            if (!sep_result) {
                // Element without separator - don't include it
                break;
            }

            results.push_back(std::move(*elem_result));
            current = sep_pos;
        }

        return {current, std::make_optional(std::move(results))};
    }

    auto operator()(std::string_view input) const
        -> std::pair<std::string_view::iterator, std::optional<output_type>>
    {
        return parse(input.begin(), input.end());
    }
};

// ============================================================================
// Factory Functions
// ============================================================================

/**
 * @brief Create a sepBy combinator
 *
 * Usage:
 *   auto csv_parser = sepBy(integer_parser(), char_parser(','));
 *   auto [pos, result] = csv_parser.parse(input.begin(), input.end());
 *   if (result) {
 *       for (auto& num : *result) { ... }
 *   }
 */
template<typename ElementParser, typename SeparatorParser>
auto sepBy(ElementParser element_parser, SeparatorParser separator_parser) {
    return SepByParser<ElementParser, SeparatorParser>(
        std::move(element_parser), std::move(separator_parser)
    );
}

/**
 * @brief Create a sepBy1 combinator (requires at least one element)
 */
template<typename ElementParser, typename SeparatorParser>
auto sepBy1(ElementParser element_parser, SeparatorParser separator_parser) {
    return SepBy1Parser<ElementParser, SeparatorParser>(
        std::move(element_parser), std::move(separator_parser)
    );
}

/**
 * @brief Create a sepEndBy combinator (allows optional trailing separator)
 */
template<typename ElementParser, typename SeparatorParser>
auto sepEndBy(ElementParser element_parser, SeparatorParser separator_parser) {
    return SepEndByParser<ElementParser, SeparatorParser>(
        std::move(element_parser), std::move(separator_parser)
    );
}

/**
 * @brief Create an endBy combinator (separator required after each element)
 */
template<typename ElementParser, typename SeparatorParser>
auto endBy(ElementParser element_parser, SeparatorParser separator_parser) {
    return EndByParser<ElementParser, SeparatorParser>(
        std::move(element_parser), std::move(separator_parser)
    );
}

// ============================================================================
// Common Separator Parsers
// ============================================================================

/**
 * @brief Simple character parser for separators
 */
class CharParser {
public:
    char target;

    explicit CharParser(char c) : target(c) {}

    using output_type = char;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<char>>
    {
        if (begin != end && *begin == target) {
            return {std::next(begin), std::make_optional(target)};
        }
        return {begin, std::nullopt};
    }

    auto operator()(std::string_view input) const
        -> std::pair<std::string_view::iterator, std::optional<char>>
    {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Factory for character parser
 */
inline auto char_parser(char c) {
    return CharParser(c);
}

/**
 * @brief Whitespace parser (spaces and tabs)
 */
class WhitespaceParser {
public:
    using output_type = std::string;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        Iterator current = begin;
        std::string spaces;

        while (current != end && (*current == ' ' || *current == '\t')) {
            spaces += *current;
            ++current;
        }

        if (spaces.empty()) {
            return {begin, std::nullopt};
        }

        return {current, std::make_optional(std::move(spaces))};
    }

    auto operator()(std::string_view input) const
        -> std::pair<std::string_view::iterator, std::optional<std::string>>
    {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Factory for whitespace parser
 */
inline auto whitespace() {
    return WhitespaceParser{};
}

/**
 * @brief Optional whitespace (always succeeds)
 */
class OptionalWhitespaceParser {
public:
    using output_type = std::string;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        Iterator current = begin;
        std::string spaces;

        while (current != end && (*current == ' ' || *current == '\t')) {
            spaces += *current;
            ++current;
        }

        return {current, std::make_optional(std::move(spaces))};
    }

    auto operator()(std::string_view input) const
        -> std::pair<std::string_view::iterator, std::optional<std::string>>
    {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Factory for optional whitespace parser
 */
inline auto optional_whitespace() {
    return OptionalWhitespaceParser{};
}

} // namespace combinators
} // namespace alga

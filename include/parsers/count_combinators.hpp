#pragma once

#include <optional>
#include <vector>
#include <utility>

namespace alga {
namespace combinators {

/**
 * @brief Count combinator - parse exactly N occurrences
 *
 * Applies a parser exactly N times. Fails if fewer than N matches found.
 *
 * Example:
 *   auto three_digits = count(3, digit_parser());
 *   auto [pos, result] = three_digits.parse("123abc");
 *   // result == vector{'1', '2', '3'}
 */
template<typename Parser>
class CountParser {
public:
    using element_type = typename Parser::output_type;
    using output_type = std::vector<element_type>;

    Parser parser;
    size_t n;

    CountParser(size_t count, Parser p) : parser(std::move(p)), n(count) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        results.reserve(n);
        Iterator current = begin;

        for (size_t i = 0; i < n; ++i) {
            auto [pos, result] = parser.parse(current, end);
            if (!result) {
                // Failed to get required number of matches
                return {begin, std::nullopt};
            }
            results.push_back(std::move(*result));
            current = pos;
        }

        return {current, std::make_optional(std::move(results))};
    }

    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Create a count parser (exactly N)
 */
template<typename Parser>
CountParser<Parser> count(size_t n, Parser parser) {
    return CountParser<Parser>(n, std::move(parser));
}

/**
 * @brief Between combinator - parse between min and max occurrences
 *
 * Applies a parser between min and max times (inclusive).
 * Fails if fewer than min matches found.
 * Succeeds with min to max matches.
 *
 * Example:
 *   auto two_to_four_digits = between(2, 4, digit_parser());
 *   auto [pos, result] = two_to_four_digits.parse("123abc");
 *   // result == vector{'1', '2', '3'} (3 is between 2 and 4)
 */
template<typename Parser>
class BetweenParser {
public:
    using element_type = typename Parser::output_type;
    using output_type = std::vector<element_type>;

    Parser parser;
    size_t min_count;
    size_t max_count;

    BetweenParser(size_t min, size_t max, Parser p)
        : parser(std::move(p)), min_count(min), max_count(max) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        results.reserve(min_count);
        Iterator current = begin;

        // First, try to match minimum required
        for (size_t i = 0; i < min_count; ++i) {
            auto [pos, result] = parser.parse(current, end);
            if (!result) {
                // Failed to get minimum required matches
                return {begin, std::nullopt};
            }
            results.push_back(std::move(*result));
            current = pos;
        }

        // Then try to match up to maximum (greedy)
        for (size_t i = min_count; i < max_count; ++i) {
            auto [pos, result] = parser.parse(current, end);
            if (!result) {
                // Stop here, we have enough
                break;
            }
            results.push_back(std::move(*result));
            current = pos;
        }

        return {current, std::make_optional(std::move(results))};
    }

    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Create a between parser (min to max inclusive)
 */
template<typename Parser>
BetweenParser<Parser> between(size_t min, size_t max, Parser parser) {
    return BetweenParser<Parser>(min, max, std::move(parser));
}

/**
 * @brief AtLeast combinator - parse at least N occurrences
 *
 * Applies a parser at least N times, then as many additional times as possible.
 * Fails if fewer than N matches found.
 *
 * Example:
 *   auto at_least_two = atLeast(2, digit_parser());
 *   auto [pos, result] = at_least_two.parse("12345abc");
 *   // result == vector{'1', '2', '3', '4', '5'} (5 >= 2)
 */
template<typename Parser>
class AtLeastParser {
public:
    using element_type = typename Parser::output_type;
    using output_type = std::vector<element_type>;

    Parser parser;
    size_t min_count;

    AtLeastParser(size_t min, Parser p) : parser(std::move(p)), min_count(min) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        results.reserve(min_count);
        Iterator current = begin;

        // First, try to match minimum required
        for (size_t i = 0; i < min_count; ++i) {
            auto [pos, result] = parser.parse(current, end);
            if (!result) {
                // Failed to get minimum required matches
                return {begin, std::nullopt};
            }
            results.push_back(std::move(*result));
            current = pos;
        }

        // Then match as many as possible (greedy, like many)
        while (current != end) {
            auto [pos, result] = parser.parse(current, end);
            if (!result) {
                break;
            }
            results.push_back(std::move(*result));
            current = pos;
        }

        return {current, std::make_optional(std::move(results))};
    }

    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Create an atLeast parser (minimum N, unbounded maximum)
 */
template<typename Parser>
AtLeastParser<Parser> atLeast(size_t min, Parser parser) {
    return AtLeastParser<Parser>(min, std::move(parser));
}

/**
 * @brief AtMost combinator - parse at most N occurrences
 *
 * Applies a parser at most N times (0 to N inclusive).
 * Always succeeds, even with 0 matches.
 *
 * Example:
 *   auto up_to_three = atMost(3, digit_parser());
 *   auto [pos, result] = up_to_three.parse("12abc");
 *   // result == vector{'1', '2'} (2 <= 3)
 */
template<typename Parser>
class AtMostParser {
public:
    using element_type = typename Parser::output_type;
    using output_type = std::vector<element_type>;

    Parser parser;
    size_t max_count;

    AtMostParser(size_t max, Parser p) : parser(std::move(p)), max_count(max) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        results.reserve(max_count);
        Iterator current = begin;

        // Match up to maximum (greedy)
        for (size_t i = 0; i < max_count && current != end; ++i) {
            auto [pos, result] = parser.parse(current, end);
            if (!result) {
                break;
            }
            results.push_back(std::move(*result));
            current = pos;
        }

        // Always succeed, even with empty vector
        return {current, std::make_optional(std::move(results))};
    }

    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Create an atMost parser (maximum N, minimum 0)
 */
template<typename Parser>
AtMostParser<Parser> atMost(size_t max, Parser parser) {
    return AtMostParser<Parser>(max, std::move(parser));
}

/**
 * @brief Times combinator - alias for count with more natural syntax
 *
 * Example:
 *   auto parser = times(5, digit_parser());  // exactly 5 digits
 */
template<typename Parser>
CountParser<Parser> times(size_t n, Parser parser) {
    return count(n, std::move(parser));
}

/**
 * @brief Exactly combinator - alias for count with more natural syntax
 *
 * Example:
 *   auto parser = exactly(3, letter_parser());  // exactly 3 letters
 */
template<typename Parser>
CountParser<Parser> exactly(size_t n, Parser parser) {
    return count(n, std::move(parser));
}

} // namespace combinators
} // namespace alga

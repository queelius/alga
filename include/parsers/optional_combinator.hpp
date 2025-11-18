#pragma once

#include <optional>
#include <utility>

namespace alga {
namespace combinators {

/**
 * @brief Optional combinator - makes any parser always succeed
 *
 * Wraps a parser to always return success, with nullopt if the inner parser fails.
 * This is useful for optional elements in parsing.
 *
 * Example:
 *   auto opt_int = optional(int_parser());
 *   auto [pos, result] = opt_int.parse("abc");
 *   // result == optional<optional<int>>(nullopt)  -- always succeeds
 *   // pos == input.begin()  -- no input consumed on failure
 */
template<typename Parser>
class OptionalParser {
public:
    using element_type = typename Parser::output_type;
    using output_type = std::optional<element_type>;

    Parser parser;

    explicit OptionalParser(Parser p) : parser(std::move(p)) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        auto [pos, result] = parser.parse(begin, end);

        if (result) {
            // Inner parser succeeded - wrap in optional
            return {pos, std::make_optional(std::make_optional(std::move(*result)))};
        } else {
            // Inner parser failed - return nullopt wrapped in optional (always succeeds)
            return {begin, std::make_optional(output_type{})};  // output_type{} is nullopt
        }
    }

    // String view convenience
    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Create an optional parser
 *
 * Makes any parser always succeed by returning nullopt on failure.
 */
template<typename Parser>
OptionalParser<Parser> optional(Parser parser) {
    return OptionalParser<Parser>(std::move(parser));
}

/**
 * @brief Many combinator - zero or more occurrences
 *
 * Applies a parser repeatedly until it fails, collecting results.
 * Always succeeds (returns empty vector if no matches).
 *
 * Example:
 *   auto digits = many(digit_parser());
 *   auto [pos, result] = digits.parse("123abc");
 *   // result == vector{'1', '2', '3'}
 */
template<typename Parser>
class ManyParser {
public:
    using element_type = typename Parser::output_type;
    using output_type = std::vector<element_type>;

    Parser parser;

    explicit ManyParser(Parser p) : parser(std::move(p)) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        Iterator current = begin;

        while (current != end) {
            auto [pos, result] = parser.parse(current, end);
            if (!result) {
                // Parser failed - stop collecting
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
 * @brief Create a many parser (zero or more)
 */
template<typename Parser>
ManyParser<Parser> many(Parser parser) {
    return ManyParser<Parser>(std::move(parser));
}

/**
 * @brief Many1 combinator - one or more occurrences
 *
 * Like many, but requires at least one match (fails if zero matches).
 *
 * Example:
 *   auto digits = many1(digit_parser());
 *   auto [pos, result] = digits.parse("abc");
 *   // result == nullopt  -- no matches, fails
 */
template<typename Parser>
class Many1Parser {
public:
    using element_type = typename Parser::output_type;
    using output_type = std::vector<element_type>;

    Parser parser;

    explicit Many1Parser(Parser p) : parser(std::move(p)) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<output_type>>
    {
        output_type results;
        Iterator current = begin;

        // Must match at least once
        auto [first_pos, first_result] = parser.parse(current, end);
        if (!first_result) {
            return {begin, std::nullopt};  // Fail if no first match
        }

        results.push_back(std::move(*first_result));
        current = first_pos;

        // Then match zero or more additional times
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
 * @brief Create a many1 parser (one or more)
 */
template<typename Parser>
Many1Parser<Parser> many1(Parser parser) {
    return Many1Parser<Parser>(std::move(parser));
}

/**
 * @brief Skip combinator - parse but discard result
 *
 * Parses with the given parser but discards the result.
 * Useful for ignoring separators or whitespace.
 *
 * Example:
 *   auto skip_ws = skip(whitespace());
 */
template<typename Parser>
class SkipParser {
public:
    using output_type = bool;  // Just indicates success

    Parser parser;

    explicit SkipParser(Parser p) : parser(std::move(p)) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<bool>>
    {
        auto [pos, result] = parser.parse(begin, end);

        if (result) {
            return {pos, std::make_optional(true)};
        } else {
            return {begin, std::nullopt};
        }
    }

    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Create a skip parser
 */
template<typename Parser>
SkipParser<Parser> skip(Parser parser) {
    return SkipParser<Parser>(std::move(parser));
}

} // namespace combinators
} // namespace alga

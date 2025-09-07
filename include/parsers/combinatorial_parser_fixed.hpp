#pragma once

/**
 * @file combinatorial_parser_fixed.hpp
 * @brief Fixed and enhanced combinatorial parser framework
 * 
 * This file provides a corrected implementation of the combinatorial parser
 * framework with proper type safety, algebraic composition, and iterator support.
 */

#include <utility>
#include <optional>
#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#include <iterator>

using std::pair;
using std::optional;
using std::string;
using std::vector;

namespace alga::combinatorial {

    /**
     * @brief Base parser interface that defines the algebraic structure
     * 
     * A parser transforms an input sequence into an optional output value,
     * consuming some portion of the input. This models the mathematical
     * concept of a partial function with explicit failure handling.
     * 
     * @tparam Input The input type (typically an iterator or string)
     * @tparam Output The output type produced by successful parsing
     */
    template<typename Input, typename Output>
    struct Parser {
        using input_type = Input;
        using output_type = Output;
        using result_type = pair<Input, optional<Output>>;
        
        /**
         * @brief Parse function that attempts to consume input and produce output
         * @param begin Iterator to start of input
         * @param end Iterator to end of input
         * @return Pair of (new_position, optional_result)
         */
        template<typename Iterator>
        result_type operator()(Iterator begin, Iterator end) const {
            return static_cast<Parser const*>(this)->parse_impl(begin, end);
        }
        
        // Default implementations for common operations
        virtual ~Parser() = default;
    };

    /**
     * @brief Chain parser that models sequential composition
     * 
     * A chain parser has no branching - it applies transformations sequentially.
     * This implements the concept of function composition in the category of parsers.
     */
    template<typename InputIter, typename OutputType>
    class ChainParser : public Parser<InputIter, OutputType> {
    public:
        using input_iterator = InputIter;
        using output_type = OutputType;
        using parse_function = std::function<pair<InputIter, optional<OutputType>>(InputIter, InputIter)>;
        
        explicit ChainParser(parse_function func) : parse_fn(std::move(func)) {}
        
        template<typename Iterator>
        pair<Iterator, optional<OutputType>> parse_impl(Iterator begin, Iterator end) const {
            return parse_fn(begin, end);
        }
        
    private:
        parse_function parse_fn;
    };

    /**
     * @brief Exclusive-or parser that implements choice composition
     * 
     * This parser tries the first option, and if it fails, tries the second.
     * This models the sum type in the algebraic structure of parsers.
     */
    template<typename FirstParser, typename SecondParser, typename OutputType>
    class ExclusiveOrParser {
    public:
        using input_type = typename FirstParser::input_type;
        using output_type = OutputType;
        
        FirstParser first;
        SecondParser second;
        
        ExclusiveOrParser(FirstParser f, SecondParser s) 
            : first(std::move(f)), second(std::move(s)) {}
        
        template<typename Iterator>
        auto operator()(Iterator begin, Iterator end) const -> pair<Iterator, optional<OutputType>> {
            // Try first parser
            auto [next1, result1] = first(begin, end);
            if (result1) {
                return {next1, optional<OutputType>{static_cast<OutputType>(*result1)}};
            }
            
            // Try second parser
            auto [next2, result2] = second(begin, end);
            if (result2) {
                return {next2, optional<OutputType>{static_cast<OutputType>(*result2)}};
            }
            
            // Both failed
            return {begin, std::nullopt};
        }
    };

    /**
     * @brief Many parser that implements Kleene star (zero or more repetitions)
     * 
     * This parser applies another parser repeatedly until it fails,
     * collecting all successful results.
     */
    template<typename BaseParser>
    class ManyParser {
    public:
        using input_type = typename BaseParser::input_type;
        using element_type = typename BaseParser::output_type;
        using output_type = vector<element_type>;
        
        BaseParser base_parser;
        
        explicit ManyParser(BaseParser parser) : base_parser(std::move(parser)) {}
        
        template<typename Iterator>
        pair<Iterator, optional<output_type>> operator()(Iterator begin, Iterator end) const {
            output_type results;
            Iterator current = begin;
            
            while (current != end) {
                auto [next, result] = base_parser(current, end);
                if (!result) {
                    break; // Parser failed, stop collecting
                }
                
                results.push_back(*result);
                
                // Ensure we're making progress
                if (next == current) {
                    break; // Prevent infinite loops on empty matches
                }
                current = next;
            }
            
            return {current, optional<output_type>{std::move(results)}};
        }
    };

    /**
     * @brief Sequential parser that applies parsers in sequence
     * 
     * This implements the product type in the algebraic structure,
     * combining the results of multiple parsers into a tuple.
     */
    template<typename Parser1, typename Parser2>
    class SequenceParser {
    public:
        using input_type = typename Parser1::input_type;
        using output_type = pair<typename Parser1::output_type, typename Parser2::output_type>;
        
        static_assert(std::is_same_v<typename Parser1::input_type, typename Parser2::input_type>,
                     "Both parsers must operate on the same input type");
        
        Parser1 first;
        Parser2 second;
        
        SequenceParser(Parser1 p1, Parser2 p2) : first(std::move(p1)), second(std::move(p2)) {}
        
        template<typename Iterator>
        pair<Iterator, optional<output_type>> operator()(Iterator begin, Iterator end) const {
            auto [next1, result1] = first(begin, end);
            if (!result1) {
                return {begin, std::nullopt};
            }
            
            auto [next2, result2] = second(next1, end);
            if (!result2) {
                return {begin, std::nullopt};
            }
            
            return {next2, optional<output_type>{std::make_pair(*result1, *result2)}};
        }
    };

    /**
     * @brief Token parser that extracts tokens based on a predicate
     * 
     * This is a fundamental parser that recognizes sequences of characters
     * satisfying a given predicate.
     */
    template<typename Predicate, typename TokenType = string>
    class TokenParser {
    public:
        using input_type = string::const_iterator;
        using output_type = TokenType;
        
        Predicate predicate;
        std::function<TokenType(string)> converter;
        
        TokenParser(Predicate pred, std::function<TokenType(string)> conv = [](string s) { return TokenType{s}; })
            : predicate(std::move(pred)), converter(std::move(conv)) {}
        
        template<typename Iterator>
        pair<Iterator, optional<output_type>> operator()(Iterator begin, Iterator end) const {
            string token;
            Iterator current = begin;
            
            while (current != end && predicate(*current)) {
                token += *current;
                ++current;
            }
            
            if (token.empty()) {
                return {begin, std::nullopt};
            }
            
            return {current, optional<output_type>{converter(std::move(token))}};
        }
    };

    // ============================================================================
    // Factory Functions for Common Parsers
    // ============================================================================

    // Predicate types for C++17 compatibility
    struct AlphaPredicate {
        bool operator()(char c) const { return std::isalpha(c); }
    };
    
    struct DigitPredicate {
        bool operator()(char c) const { return std::isdigit(c); }
    };
    
    struct SpacePredicate {
        bool operator()(char c) const { return std::isspace(c); }
    };

    /**
     * @brief Create a parser that recognizes alphabetic characters
     */
    auto make_alpha_parser() {
        return TokenParser<AlphaPredicate, string>{
            AlphaPredicate{},
            [](string s) { 
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                return s; 
            }
        };
    }

    /**
     * @brief Create a parser that recognizes numeric characters
     */
    auto make_digit_parser() {
        return TokenParser<DigitPredicate, int>{
            DigitPredicate{},
            [](string s) { return std::stoi(s); }
        };
    }

    /**
     * @brief Create a parser that recognizes whitespace
     */
    auto make_whitespace_parser() {
        return TokenParser<SpacePredicate, string>{
            SpacePredicate{}
        };
    }

    /**
     * @brief Create a literal parser that matches a specific string
     */
    class LiteralParser {
    public:
        using input_type = string::const_iterator;
        using output_type = string;
        
        string literal;
        
        explicit LiteralParser(string lit) : literal(std::move(lit)) {}
        
        template<typename Iterator>
        pair<Iterator, optional<output_type>> operator()(Iterator begin, Iterator end) const {
            if (std::distance(begin, end) < static_cast<long>(literal.length())) {
                return {begin, std::nullopt};
            }
            
            if (std::equal(literal.begin(), literal.end(), begin)) {
                return {std::next(begin, literal.length()), optional<output_type>{literal}};
            }
            
            return {begin, std::nullopt};
        }
    };

    /**
     * @brief Create a literal parser
     */
    auto literal(string s) {
        return LiteralParser{std::move(s)};
    }

    // ============================================================================
    // Combinator Functions
    // ============================================================================

    /**
     * @brief Sequence combinator - applies parsers in order
     */
    template<typename P1, typename P2>
    auto sequence(P1&& p1, P2&& p2) {
        return SequenceParser<std::decay_t<P1>, std::decay_t<P2>>{
            std::forward<P1>(p1), std::forward<P2>(p2)
        };
    }

    /**
     * @brief Alternative combinator - tries first parser, then second
     */
    template<typename P1, typename P2, typename OutputType = typename P1::output_type>
    auto alternative(P1&& p1, P2&& p2) {
        return ExclusiveOrParser<std::decay_t<P1>, std::decay_t<P2>, OutputType>{
            std::forward<P1>(p1), std::forward<P2>(p2)
        };
    }

    /**
     * @brief Many combinator - zero or more repetitions
     */
    template<typename Parser>
    auto many(Parser&& parser) {
        return ManyParser<std::decay_t<Parser>>{std::forward<Parser>(parser)};
    }

    /**
     * @brief Many1 combinator - one or more repetitions
     */
    template<typename Parser>
    auto many1(Parser&& parser) {
        return sequence(parser, many(parser));
    }

    /**
     * @brief Optional combinator - parser that always succeeds
     */
    template<typename Parser>
    class OptionalParser {
    public:
        using input_type = typename Parser::input_type;
        using output_type = optional<typename Parser::output_type>;
        
        Parser base_parser;
        
        explicit OptionalParser(Parser p) : base_parser(std::move(p)) {}
        
        template<typename Iterator>
        pair<Iterator, optional<output_type>> operator()(Iterator begin, Iterator end) const {
            auto [next, result] = base_parser(begin, end);
            if (result) {
                return {next, optional<output_type>{*result}};
            } else {
                return {begin, optional<output_type>{std::nullopt}};
            }
        }
    };

    template<typename Parser>
    auto maybe(Parser&& parser) {  // Renamed from 'optional' to avoid std::optional conflict
        return OptionalParser<std::decay_t<Parser>>{std::forward<Parser>(parser)};
    }

    // ============================================================================
    // Higher-Order Parser Combinators
    // ============================================================================

    /**
     * @brief Transform parser - applies function to successful parse results
     */
    template<typename Parser, typename Function>
    class TransformParser {
    public:
        using input_type = typename Parser::input_type;
        using output_type = std::invoke_result_t<Function, typename Parser::output_type>;
        
        Parser base_parser;
        Function transform_fn;
        
        TransformParser(Parser p, Function f) 
            : base_parser(std::move(p)), transform_fn(std::move(f)) {}
        
        template<typename Iterator>
        pair<Iterator, optional<output_type>> operator()(Iterator begin, Iterator end) const {
            auto [next, result] = base_parser(begin, end);
            if (!result) {
                return {begin, std::nullopt};
            }
            
            return {next, optional<output_type>{transform_fn(*result)}};
        }
    };

    template<typename Parser, typename Function>
    auto transform(Parser&& parser, Function&& func) {
        return TransformParser<std::decay_t<Parser>, std::decay_t<Function>>{
            std::forward<Parser>(parser), std::forward<Function>(func)
        };
    }

    /**
     * @brief Bind parser - monadic composition
     */
    template<typename Parser, typename Function>
    class BindParser {
    public:
        using input_type = typename Parser::input_type;
        using output_type = typename std::invoke_result_t<Function, typename Parser::output_type>::output_type;
        
        Parser base_parser;
        Function continuation;
        
        BindParser(Parser p, Function f) 
            : base_parser(std::move(p)), continuation(std::move(f)) {}
        
        template<typename Iterator>
        pair<Iterator, optional<output_type>> operator()(Iterator begin, Iterator end) const {
            auto [next1, result1] = base_parser(begin, end);
            if (!result1) {
                return {begin, std::nullopt};
            }
            
            auto next_parser = continuation(*result1);
            return next_parser(next1, end);
        }
    };

    template<typename Parser, typename Function>
    auto bind(Parser&& parser, Function&& func) {
        return BindParser<std::decay_t<Parser>, std::decay_t<Function>>{
            std::forward<Parser>(parser), std::forward<Function>(func)
        };
    }

    // ============================================================================
    // Utility Functions
    // ============================================================================

    /**
     * @brief Parse a complete input with a given parser
     */
    template<typename Parser, typename Container>
    optional<typename Parser::output_type> parse(Parser&& parser, Container const& input) {
        auto [remaining, result] = parser(input.begin(), input.end());
        
        // Only return result if we consumed all input
        if (result && remaining == input.end()) {
            return *result;
        }
        
        return std::nullopt;
    }

    /**
     * @brief Parse allowing partial consumption of input
     */
    template<typename Parser, typename Container>
    pair<typename Container::const_iterator, optional<typename Parser::output_type>> 
    parse_partial(Parser&& parser, Container const& input) {
        return parser(input.begin(), input.end());
    }

} // namespace alga::combinatorial
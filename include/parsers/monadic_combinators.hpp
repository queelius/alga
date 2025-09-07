#pragma once

#include "lc_alpha.hpp"
#include "porter2stemmer.hpp"
#include "ngram_stemmer.hpp"
#include <optional>
#include <functional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <future>
#include <utility>

namespace alga {
namespace monadic {

/**
 * @brief Monadic bind operator for optional chaining
 * 
 * The fundamental building block for monadic parser composition.
 * Enables chaining operations that may fail without explicit error checking.
 */
template<typename T, typename Function>
auto operator>>=(std::optional<T> const& opt, Function f) 
    -> std::enable_if_t<
        std::is_same_v<decltype(f(*opt)), std::optional<std::decay_t<decltype(*f(*opt))>>>,
        decltype(f(*opt))
    >
{
    if (!opt) {
        return {};
    }
    return f(*opt);
}

/**
 * @brief Monadic return - lift a value into the optional context
 */
template<typename T>
constexpr std::optional<T> pure(T&& value) {
    return std::make_optional(std::forward<T>(value));
}

/**
 * @brief Applicative apply - apply a function wrapped in optional to a value in optional
 */
template<typename Function, typename T>
auto apply(std::optional<Function> const& func, std::optional<T> const& value) 
    -> std::optional<std::decay_t<decltype((*func)(*value))>>
{
    if (!func || !value) {
        return {};
    }
    return pure((*func)(*value));
}

/**
 * @brief Lift a binary function to work with optional values
 */
template<typename Function>
auto lift2(Function f) {
    return [f](auto const& a, auto const& b) -> std::optional<std::decay_t<decltype(f(*a, *b))>> {
        if (!a || !b) {
            return {};
        }
        return pure(f(*a, *b));
    };
}

/**
 * @brief Sequential composition of parsers - run parsers in sequence
 * 
 * This leverages our uniform optional pattern across all algebraic types.
 */
template<typename... Parsers>
class SequentialParser {
public:
    std::tuple<Parsers...> parsers;
    
    explicit SequentialParser(Parsers... ps) : parsers(std::move(ps)...) {}
    
    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const {
        return parse_sequence(begin, end, std::index_sequence_for<Parsers...>{});
    }
    
    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
    
private:
    template<typename Iterator, std::size_t... Is>
    auto parse_sequence(Iterator begin, Iterator end, std::index_sequence<Is...>) const {
        using ResultTuple = std::tuple<std::optional<typename Parsers::output_type>...>;
        ResultTuple results;
        Iterator current = begin;
        
        // Parse each parser in sequence, short-circuiting on failure
        bool success = ((parse_single<Is>(current, end, results) && true) && ...);
        
        if (!success) {
            return std::make_pair(begin, std::optional<ResultTuple>{});
        }
        
        return std::make_pair(current, std::make_optional(std::move(results)));
    }
    
    template<std::size_t I, typename Iterator, typename ResultTuple>
    bool parse_single(Iterator& current, Iterator end, ResultTuple& results) const {
        auto [next, result] = std::get<I>(parsers).parse(current, end);
        std::get<I>(results) = result;
        
        if (!result) {
            return false;
        }
        
        current = next;
        return true;
    }
};

/**
 * @brief Choice combinator - try parsers until one succeeds
 */
template<typename... Parsers>
class ChoiceParser {
public:
    std::tuple<Parsers...> parsers;
    
    explicit ChoiceParser(Parsers... ps) : parsers(std::move(ps)...) {}
    
    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const {
        return try_choices(begin, end, std::index_sequence_for<Parsers...>{});
    }
    
    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
    
private:
    template<typename Iterator, std::size_t... Is>
    auto try_choices(Iterator begin, Iterator end, std::index_sequence<Is...>) const {
        using FirstResultType = typename std::tuple_element_t<0, std::tuple<Parsers...>>::output_type;
        std::optional<FirstResultType> result;
        Iterator final_pos = begin;
        
        // Try each parser until one succeeds
        ((try_single<Is>(begin, end, result, final_pos)) || ...);
        
        return std::make_pair(final_pos, result);
    }
    
    template<std::size_t I, typename Iterator, typename Result>
    bool try_single(Iterator begin, Iterator end, Result& result, Iterator& final_pos) const {
        auto [next, parse_result] = std::get<I>(parsers).parse(begin, end);
        
        if (parse_result) {
            result = parse_result;
            final_pos = next;
            return true; // Success - stop trying alternatives
        }
        
        return false; // Continue trying alternatives
    }
};

/**
 * @brief Many combinator - parse zero or more occurrences
 */
template<typename Parser>
class ManyParser {
public:
    Parser parser;
    
    explicit ManyParser(Parser p) : parser(std::move(p)) {}
    
    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const {
        using ResultType = typename Parser::output_type;
        std::vector<ResultType> results;
        Iterator current = begin;
        
        while (current != end) {
            auto [next, result] = parser.parse(current, end);
            
            if (!result) {
                break; // No more matches
            }
            
            results.push_back(std::move(*result));
            current = next;
            
            // Prevent infinite loops on zero-consumption parsers
            if (current == next) {
                break;
            }
        }
        
        return std::make_pair(current, pure(std::move(results)));
    }
    
    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

/**
 * @brief Transform combinator - map a function over parser results
 */
template<typename Parser, typename Function>
class TransformParser {
public:
    Parser parser;
    Function func;
    
    TransformParser(Parser p, Function f) : parser(std::move(p)), func(std::move(f)) {}
    
    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const {
        auto [next, result] = parser.parse(begin, end);
        
        if (!result) {
            return std::make_pair(begin, std::optional<std::decay_t<decltype(func(*result))>>{});
        }
        
        return std::make_pair(next, pure(func(*result)));
    }
    
    auto operator()(std::string_view input) const {
        return parse(input.begin(), input.end());
    }
};

// Factory functions for ergonomic parser construction

template<typename... Parsers>
auto sequence(Parsers... parsers) {
    return SequentialParser<Parsers...>(std::move(parsers)...);
}

template<typename... Parsers>
auto choice(Parsers... parsers) {
    return ChoiceParser<Parsers...>(std::move(parsers)...);
}

template<typename Parser>
auto many(Parser parser) {
    return ManyParser<Parser>(std::move(parser));
}

template<typename Parser, typename Function>
auto transform(Parser parser, Function func) {
    return TransformParser<Parser, Function>(std::move(parser), std::move(func));
}

/**
 * @brief Parallel composition - run multiple parsers concurrently
 * 
 * Leverages perfect value semantics for safe parallel execution.
 */
template<typename... Parsers>
class ParallelParser {
public:
    std::tuple<Parsers...> parsers;
    
    explicit ParallelParser(Parsers... ps) : parsers(std::move(ps)...) {}
    
    auto operator()(std::string_view input) const {
        return apply_parallel(input, std::index_sequence_for<Parsers...>{});
    }
    
private:
    template<std::size_t... Is>
    auto apply_parallel(std::string_view input, std::index_sequence<Is...>) const {
        // Launch all parsers concurrently
        auto futures = std::make_tuple(
            std::async(std::launch::async, [&, this]() {
                return std::get<Is>(parsers)(input);
            })...
        );
        
        // Collect all results
        return std::make_tuple(futures.get()...);
    }
};

template<typename... Parsers>
auto parallel(Parsers... parsers) {
    return ParallelParser<Parsers...>(std::move(parsers)...);
}

/**
 * @brief Algebraic parser adapter
 * 
 * Bridges our algebraic types with the combinator framework.
 */
template<typename AlgebraicType>
class AlgebraicParser {
public:
    using output_type = AlgebraicType;
    std::function<std::optional<AlgebraicType>(std::string_view)> factory;
    
    template<typename Factory>
    explicit AlgebraicParser(Factory f) : factory(f) {}
    
    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const {
        // Extract token from iterator range (simplified - could be more sophisticated)
        std::string token;
        Iterator current = begin;
        
        while (current != end && std::isalpha(*current)) {
            token += std::tolower(*current);
            ++current;
        }
        
        if (token.empty()) {
            return std::make_pair(begin, std::optional<AlgebraicType>{});
        }
        
        auto result = factory(token);
        return std::make_pair(current, result);
    }
    
    auto operator()(std::string_view input) const {
        return factory(input);
    }
};

// Factory functions for algebraic parsers
inline auto lc_alpha_parser() {
    return AlgebraicParser<lc_alpha>([](std::string_view input) {
        return make_lc_alpha(input);
    });
}

inline auto porter2_stem_parser() {
    return AlgebraicParser<porter2_stem>([](std::string_view input) {
        return make_porter2_stem(input);
    });
}

} // namespace monadic
} // namespace alga
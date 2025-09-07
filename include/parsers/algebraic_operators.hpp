#pragma once

#include <concepts>
#include <optional>
#include <functional>
#include <type_traits>
#include <utility>
#include <future>
#include <vector>

namespace alga {

/**
 * @brief Concepts for algebraic structure requirements using C++20
 */
namespace concepts {

/**
 * @brief Concept for types that support algebraic composition
 */
template<typename T>
concept AlgebraicType = requires(T const& a, T const& b) {
    { a * b } -> std::convertible_to<T>;
    { T{} };  // Default constructible (identity element)
};

/**
 * @brief Concept for types that support optional algebraic composition
 */
template<typename T>
concept OptionalAlgebraic = AlgebraicType<T> && requires(std::optional<T> const& a, std::optional<T> const& b) {
    { a * b } -> std::convertible_to<std::optional<T>>;
};

/**
 * @brief Concept for types that can be compared
 */
template<typename T>
concept Comparable = requires(T const& a, T const& b) {
    { a == b } -> std::convertible_to<bool>;
    { a != b } -> std::convertible_to<bool>;
    { a < b } -> std::convertible_to<bool>;
};

/**
 * @brief Concept for types that support choice operations
 */
template<typename T>
concept Choiceable = AlgebraicType<T> && Comparable<T>;

/**
 * @brief Concept for function application
 */
template<typename F, typename T>
concept ApplicableFunction = requires(F const& f, T const& t) {
    { f(t) };
};

} // namespace concepts

/**
 * @brief Extended algebraic operators for compositional parsing
 */
namespace operators {

/**
 * @brief Choice operator - try left operand, if it fails/is invalid, try right
 * 
 * For optional types: returns first non-nullopt value
 * For regular types: uses comparison to determine validity/preference
 */
template<concepts::OptionalAlgebraic T>
std::optional<T> operator|(std::optional<T> const& lhs, std::optional<T> const& rhs) {
    return lhs ? lhs : rhs;
}

/**
 * @brief Choice operator for regular algebraic types
 * Uses empty() method or size() == 0 to determine validity
 */
template<concepts::Choiceable T>
    requires requires(T const& t) { t.empty(); }
T operator|(T const& lhs, T const& rhs) {
    return lhs.empty() ? rhs : lhs;
}

/**
 * @brief Multiple choice - chain multiple alternatives
 */
template<concepts::OptionalAlgebraic T>
std::optional<T> choice_of(std::vector<std::optional<T>> const& alternatives) {
    for (auto const& alt : alternatives) {
        if (alt) return alt;
    }
    return std::nullopt;
}

/**
 * @brief Repetition operator - compose element with itself N times
 * 
 * Implements: a^n = a * a * ... * a (n times)
 */
template<concepts::AlgebraicType T>
T operator^(T const& base, size_t count) {
    if (count == 0) return T{};  // Identity element
    if (count == 1) return base;
    
    T result = base;
    for (size_t i = 1; i < count; ++i) {
        result = result * base;
    }
    return result;
}

/**
 * @brief Function application operator
 * 
 * Applies a function to a value: value % function
 */
template<typename T, concepts::ApplicableFunction<T> F>
auto operator%(T const& value, F const& function) -> decltype(function(value)) {
    return function(value);
}

/**
 * @brief Function application for optional values
 * 
 * Only applies function if optional has value
 */
template<typename T, concepts::ApplicableFunction<T> F>
auto operator%(std::optional<T> const& maybe_value, F const& function) 
    -> std::optional<std::decay_t<decltype(function(*maybe_value))>> {
    if (!maybe_value) return std::nullopt;
    return function(*maybe_value);
}

/**
 * @brief Sequential composition operator (left-to-right data flow)
 * 
 * Different semantics from * - preserves order and structure
 */
template<concepts::AlgebraicType T>
std::vector<T> operator>>(T const& lhs, T const& rhs) {
    return {lhs, rhs};
}

/**
 * @brief Sequential composition for vectors
 */
template<concepts::AlgebraicType T>
std::vector<T> operator>>(std::vector<T> const& lhs, T const& rhs) {
    auto result = lhs;
    result.push_back(rhs);
    return result;
}

/**
 * @brief Data flow operator (right-to-left composition)
 */
template<typename T, concepts::ApplicableFunction<T> F>
auto operator<<(F const& function, T const& value) -> decltype(function(value)) {
    return function(value);
}

/**
 * @brief Logical OR for optionals - first non-nullopt wins
 */
template<concepts::OptionalAlgebraic T>
std::optional<T> operator||(std::optional<T> const& lhs, std::optional<T> const& rhs) {
    return lhs ? lhs : rhs;
}

/**
 * @brief Logical AND for optionals - both must succeed
 */
template<concepts::OptionalAlgebraic T>
std::optional<std::pair<T, T>> operator&&(std::optional<T> const& lhs, std::optional<T> const& rhs) {
    if (!lhs || !rhs) return std::nullopt;
    return std::make_pair(*lhs, *rhs);
}

/**
 * @brief Union operator for combining results
 */
template<concepts::AlgebraicType T>
T operator+(T const& lhs, T const& rhs) {
    return lhs * rhs;  // Default to composition
}

/**
 * @brief Parallel composition operator
 * 
 * Evaluates multiple operations in parallel and combines results
 */
template<concepts::AlgebraicType T>
std::future<std::vector<T>> operator&(std::function<T()> const& lhs, std::function<T()> const& rhs) {
    return std::async(std::launch::async, [lhs, rhs]() {
        auto future1 = std::async(std::launch::async, lhs);
        auto future2 = std::async(std::launch::async, rhs);
        return std::vector<T>{future1.get(), future2.get()};
    });
}

/**
 * @brief Lifting operator - lift a function to work on optional values
 * 
 * Creates a lifted version of a function that works on optionals
 */
template<typename T, typename U>
auto lift(std::function<U(T)> const& f) {
    return [f](std::optional<T> const& maybe_t) -> std::optional<U> {
        if (!maybe_t) return std::nullopt;
        return f(*maybe_t);
    };
}

/**
 * @brief Operator~ for convenient lifting syntax
 */
template<typename F>
auto operator~(F const& function) {
    return [function](auto const& maybe_value) -> decltype(lift(std::function{function})(maybe_value)) {
        return lift(std::function{function})(maybe_value);
    };
}

} // namespace operators

// Bring operators into the main namespace for convenience
using namespace operators;

} // namespace alga
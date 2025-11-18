#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <iostream>
#include <cmath>
#include <limits>
#include <charconv>
#include <algorithm>

using std::string;
using std::string_view;
using std::optional;

namespace alga
{
    // ============================================================================
    // unsigned_int: Non-negative integers with overflow checking
    // ============================================================================

    /**
     * @brief Type-safe unsigned integers with proper value semantics
     *
     * Forms a monoid under addition with 0 as identity.
     * Supports full value semantics and algebraic composition.
     */
    class unsigned_int
    {
    private:
        uint64_t value;

        // Private constructor maintains invariant
        explicit unsigned_int(uint64_t val) : value(val) {}

    public:
        // Factory function is the ONLY way to create from potentially invalid input
        friend optional<unsigned_int> make_unsigned_int(string_view input);
        friend optional<unsigned_int> make_unsigned_int(uint64_t val);

        // Monoid operations (addition)
        friend unsigned_int operator*(unsigned_int const& lhs, unsigned_int const& rhs);
        friend optional<unsigned_int> operator*(optional<unsigned_int> const& lhs, optional<unsigned_int> const& rhs);

        // Arithmetic operations
        friend unsigned_int operator+(unsigned_int const& lhs, unsigned_int const& rhs);
        friend optional<unsigned_int> operator+(optional<unsigned_int> const& lhs, optional<unsigned_int> const& rhs);

        // Extended algebraic operators
        friend unsigned_int operator|(unsigned_int const& lhs, unsigned_int const& rhs);
        friend unsigned_int operator^(unsigned_int const& base, size_t count);

        // Comparison operators
        friend bool operator==(unsigned_int const& lhs, unsigned_int const& rhs);
        friend bool operator!=(unsigned_int const& lhs, unsigned_int const& rhs);
        friend bool operator<(unsigned_int const& lhs, unsigned_int const& rhs);
        friend bool operator<=(unsigned_int const& lhs, unsigned_int const& rhs);
        friend bool operator>(unsigned_int const& lhs, unsigned_int const& rhs);
        friend bool operator>=(unsigned_int const& lhs, unsigned_int const& rhs);

        // Stream output
        friend std::ostream& operator<<(std::ostream& os, unsigned_int const& ui);

        // Default constructible (identity element = 0)
        unsigned_int() : value(0) {}

        // Full value semantics
        unsigned_int(unsigned_int const& other) = default;
        unsigned_int(unsigned_int&& other) = default;
        unsigned_int& operator=(unsigned_int const& other) = default;
        unsigned_int& operator=(unsigned_int&& other) = default;

        // Access interface
        uint64_t val() const { return value; }
        explicit operator uint64_t() const { return value; }
        string str() const { return std::to_string(value); }
        explicit operator string() const { return str(); }

        // Utility
        bool empty() const { return value == 0; }
    };

    /**
     * @brief Validate and create unsigned_int from string input
     */
    optional<unsigned_int> make_unsigned_int(string_view input)
    {
        if (input.empty()) {
            return std::nullopt;
        }

        // Validate: only digits allowed
        if (!std::all_of(input.begin(), input.end(), [](char c) { return std::isdigit(c); })) {
            return std::nullopt;
        }

        // Parse using std::from_chars for safety
        uint64_t result;
        auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), result);

        if (ec == std::errc() && ptr == input.data() + input.size()) {
            return unsigned_int(result);
        }

        return std::nullopt;  // Overflow or parse error
    }

    /**
     * @brief Create unsigned_int directly from uint64_t value
     */
    optional<unsigned_int> make_unsigned_int(uint64_t val)
    {
        return unsigned_int(val);
    }

    // Monoid operation: addition
    unsigned_int operator*(unsigned_int const& lhs, unsigned_int const& rhs)
    {
        // Check for overflow
        if (lhs.value > std::numeric_limits<uint64_t>::max() - rhs.value) {
            return unsigned_int(std::numeric_limits<uint64_t>::max());  // Saturate
        }
        return unsigned_int(lhs.value + rhs.value);
    }

    optional<unsigned_int> operator*(optional<unsigned_int> const& lhs, optional<unsigned_int> const& rhs)
    {
        if (!lhs || !rhs) return std::nullopt;
        return *lhs * *rhs;
    }

    // Arithmetic addition (same as monoid operation)
    unsigned_int operator+(unsigned_int const& lhs, unsigned_int const& rhs)
    {
        return lhs * rhs;
    }

    optional<unsigned_int> operator+(optional<unsigned_int> const& lhs, optional<unsigned_int> const& rhs)
    {
        return lhs * rhs;
    }

    // Comparison operators
    bool operator==(unsigned_int const& lhs, unsigned_int const& rhs) { return lhs.value == rhs.value; }
    bool operator!=(unsigned_int const& lhs, unsigned_int const& rhs) { return !(lhs == rhs); }
    bool operator<(unsigned_int const& lhs, unsigned_int const& rhs) { return lhs.value < rhs.value; }
    bool operator<=(unsigned_int const& lhs, unsigned_int const& rhs) { return lhs.value <= rhs.value; }
    bool operator>(unsigned_int const& lhs, unsigned_int const& rhs) { return lhs.value > rhs.value; }
    bool operator>=(unsigned_int const& lhs, unsigned_int const& rhs) { return lhs.value >= rhs.value; }

    // Stream output
    std::ostream& operator<<(std::ostream& os, unsigned_int const& ui)
    {
        return os << ui.value;
    }

    // Choice operator
    unsigned_int operator|(unsigned_int const& lhs, unsigned_int const& rhs)
    {
        return lhs.empty() ? rhs : lhs;
    }

    optional<unsigned_int> operator|(optional<unsigned_int> const& lhs, optional<unsigned_int> const& rhs)
    {
        return lhs ? lhs : rhs;
    }

    // Repetition operator (multiplication by scalar)
    unsigned_int operator^(unsigned_int const& base, size_t count)
    {
        if (count == 0) return unsigned_int{};
        if (count == 1) return base;

        uint64_t result = base.value;
        for (size_t i = 1; i < count; ++i) {
            if (result > std::numeric_limits<uint64_t>::max() - base.value) {
                return unsigned_int(std::numeric_limits<uint64_t>::max());  // Saturate
            }
            result += base.value;
        }
        return unsigned_int(result);
    }

    // Function application operator
    template<typename F>
        requires std::invocable<F, unsigned_int>
    auto operator%(unsigned_int const& value, F const& function) -> decltype(function(value))
    {
        return function(value);
    }

    template<typename F>
        requires std::invocable<F, unsigned_int>
    auto operator%(optional<unsigned_int> const& maybe_value, F const& function)
        -> optional<std::decay_t<decltype(function(*maybe_value))>>
    {
        if (!maybe_value) return std::nullopt;
        return function(*maybe_value);
    }

    // ============================================================================
    // signed_int: Positive and negative integers
    // ============================================================================

    /**
     * @brief Type-safe signed integers with proper value semantics
     *
     * Forms a monoid under addition with 0 as identity.
     */
    class signed_int
    {
    private:
        int64_t value;

        explicit signed_int(int64_t val) : value(val) {}

    public:
        friend optional<signed_int> make_signed_int(string_view input);
        friend optional<signed_int> make_signed_int(int64_t val);

        // Monoid operations
        friend signed_int operator*(signed_int const& lhs, signed_int const& rhs);
        friend optional<signed_int> operator*(optional<signed_int> const& lhs, optional<signed_int> const& rhs);

        // Arithmetic operations
        friend signed_int operator+(signed_int const& lhs, signed_int const& rhs);
        friend optional<signed_int> operator+(optional<signed_int> const& lhs, optional<signed_int> const& rhs);
        friend signed_int operator-(signed_int const& lhs, signed_int const& rhs);

        // Extended operators
        friend signed_int operator|(signed_int const& lhs, signed_int const& rhs);
        friend signed_int operator^(signed_int const& base, size_t count);

        // Comparison operators
        friend bool operator==(signed_int const& lhs, signed_int const& rhs);
        friend bool operator!=(signed_int const& lhs, signed_int const& rhs);
        friend bool operator<(signed_int const& lhs, signed_int const& rhs);
        friend bool operator<=(signed_int const& lhs, signed_int const& rhs);
        friend bool operator>(signed_int const& lhs, signed_int const& rhs);
        friend bool operator>=(signed_int const& lhs, signed_int const& rhs);

        // Stream output
        friend std::ostream& operator<<(std::ostream& os, signed_int const& si);

        // Default constructible (identity = 0)
        signed_int() : value(0) {}

        // Full value semantics
        signed_int(signed_int const& other) = default;
        signed_int(signed_int&& other) = default;
        signed_int& operator=(signed_int const& other) = default;
        signed_int& operator=(signed_int&& other) = default;

        // Access interface
        int64_t val() const { return value; }
        explicit operator int64_t() const { return value; }
        string str() const { return std::to_string(value); }
        explicit operator string() const { return str(); }

        bool empty() const { return value == 0; }
    };

    /**
     * @brief Validate and create signed_int from string input
     */
    optional<signed_int> make_signed_int(string_view input)
    {
        if (input.empty()) {
            return std::nullopt;
        }

        // Validate format: optional sign followed by digits
        size_t start_pos = 0;
        bool has_plus = false;
        if (input[0] == '+' || input[0] == '-') {
            has_plus = (input[0] == '+');
            start_pos = 1;
            if (input.size() == 1) return std::nullopt;  // Just a sign, no digits
        }

        if (!std::all_of(input.begin() + start_pos, input.end(), [](char c) { return std::isdigit(c); })) {
            return std::nullopt;
        }

        int64_t result;
        // std::from_chars doesn't handle '+' prefix, so skip it if present
        auto parse_start = has_plus ? input.data() + 1 : input.data();
        auto [ptr, ec] = std::from_chars(parse_start, input.data() + input.size(), result);

        if (ec == std::errc() && ptr == input.data() + input.size()) {
            return signed_int(result);
        }

        return std::nullopt;
    }

    optional<signed_int> make_signed_int(int64_t val)
    {
        return signed_int(val);
    }

    // Monoid operation: addition
    signed_int operator*(signed_int const& lhs, signed_int const& rhs)
    {
        // Check for overflow
        if ((rhs.value > 0 && lhs.value > std::numeric_limits<int64_t>::max() - rhs.value) ||
            (rhs.value < 0 && lhs.value < std::numeric_limits<int64_t>::min() - rhs.value)) {
            // Saturate on overflow
            return signed_int(rhs.value > 0 ? std::numeric_limits<int64_t>::max() : std::numeric_limits<int64_t>::min());
        }
        return signed_int(lhs.value + rhs.value);
    }

    optional<signed_int> operator*(optional<signed_int> const& lhs, optional<signed_int> const& rhs)
    {
        if (!lhs || !rhs) return std::nullopt;
        return *lhs * *rhs;
    }

    signed_int operator+(signed_int const& lhs, signed_int const& rhs) { return lhs * rhs; }
    optional<signed_int> operator+(optional<signed_int> const& lhs, optional<signed_int> const& rhs) { return lhs * rhs; }

    signed_int operator-(signed_int const& lhs, signed_int const& rhs)
    {
        if (rhs.value == std::numeric_limits<int64_t>::min()) {
            return signed_int(std::numeric_limits<int64_t>::max());  // Saturate
        }
        return lhs * signed_int(-rhs.value);
    }

    // Comparison operators
    bool operator==(signed_int const& lhs, signed_int const& rhs) { return lhs.value == rhs.value; }
    bool operator!=(signed_int const& lhs, signed_int const& rhs) { return !(lhs == rhs); }
    bool operator<(signed_int const& lhs, signed_int const& rhs) { return lhs.value < rhs.value; }
    bool operator<=(signed_int const& lhs, signed_int const& rhs) { return lhs.value <= rhs.value; }
    bool operator>(signed_int const& lhs, signed_int const& rhs) { return lhs.value > rhs.value; }
    bool operator>=(signed_int const& lhs, signed_int const& rhs) { return lhs.value >= rhs.value; }

    std::ostream& operator<<(std::ostream& os, signed_int const& si)
    {
        return os << si.value;
    }

    signed_int operator|(signed_int const& lhs, signed_int const& rhs)
    {
        return lhs.empty() ? rhs : lhs;
    }

    optional<signed_int> operator|(optional<signed_int> const& lhs, optional<signed_int> const& rhs)
    {
        return lhs ? lhs : rhs;
    }

    signed_int operator^(signed_int const& base, size_t count)
    {
        if (count == 0) return signed_int{};
        if (count == 1) return base;

        int64_t result = base.value * static_cast<int64_t>(count);
        return signed_int(result);
    }

    template<typename F>
        requires std::invocable<F, signed_int>
    auto operator%(signed_int const& value, F const& function) -> decltype(function(value))
    {
        return function(value);
    }

    template<typename F>
        requires std::invocable<F, signed_int>
    auto operator%(optional<signed_int> const& maybe_value, F const& function)
        -> optional<std::decay_t<decltype(function(*maybe_value))>>
    {
        if (!maybe_value) return std::nullopt;
        return function(*maybe_value);
    }

    // ============================================================================
    // floating_point: Decimal numbers with precision
    // ============================================================================

    /**
     * @brief Type-safe floating point numbers with proper value semantics
     *
     * Forms a monoid under addition with 0.0 as identity.
     */
    class floating_point
    {
    private:
        double value;

        explicit floating_point(double val) : value(val) {}

    public:
        friend optional<floating_point> make_floating_point(string_view input);
        friend optional<floating_point> make_floating_point(double val);

        // Monoid operations
        friend floating_point operator*(floating_point const& lhs, floating_point const& rhs);
        friend optional<floating_point> operator*(optional<floating_point> const& lhs, optional<floating_point> const& rhs);

        // Arithmetic operations
        friend floating_point operator+(floating_point const& lhs, floating_point const& rhs);
        friend optional<floating_point> operator+(optional<floating_point> const& lhs, optional<floating_point> const& rhs);
        friend floating_point operator-(floating_point const& lhs, floating_point const& rhs);

        // Extended operators
        friend floating_point operator|(floating_point const& lhs, floating_point const& rhs);
        friend floating_point operator^(floating_point const& base, size_t count);

        // Comparison operators (with epsilon for floating point)
        friend bool operator==(floating_point const& lhs, floating_point const& rhs);
        friend bool operator!=(floating_point const& lhs, floating_point const& rhs);
        friend bool operator<(floating_point const& lhs, floating_point const& rhs);
        friend bool operator<=(floating_point const& lhs, floating_point const& rhs);
        friend bool operator>(floating_point const& lhs, floating_point const& rhs);
        friend bool operator>=(floating_point const& lhs, floating_point const& rhs);

        // Stream output
        friend std::ostream& operator<<(std::ostream& os, floating_point const& fp);

        // Default constructible (identity = 0.0)
        floating_point() : value(0.0) {}

        // Full value semantics
        floating_point(floating_point const& other) = default;
        floating_point(floating_point&& other) = default;
        floating_point& operator=(floating_point const& other) = default;
        floating_point& operator=(floating_point&& other) = default;

        // Access interface
        double val() const { return value; }
        explicit operator double() const { return value; }
        string str() const { return std::to_string(value); }
        explicit operator string() const { return str(); }

        bool empty() const { return std::abs(value) < 1e-10; }
    };

    /**
     * @brief Validate and create floating_point from string input
     */
    optional<floating_point> make_floating_point(string_view input)
    {
        if (input.empty()) {
            return std::nullopt;
        }

        // Validate format: [+-]?[0-9]*\.?[0-9]+
        bool has_digit = false;
        bool has_decimal = false;
        size_t i = 0;

        if (input[i] == '+' || input[i] == '-') ++i;

        for (; i < input.size(); ++i) {
            if (std::isdigit(input[i])) {
                has_digit = true;
            } else if (input[i] == '.') {
                if (has_decimal) return std::nullopt;  // Multiple decimal points
                has_decimal = true;
            } else {
                return std::nullopt;  // Invalid character
            }
        }

        if (!has_digit) return std::nullopt;

        // Parse using std::from_chars (C++17 for floating point)
        double result;
        auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), result);

        if (ec == std::errc() && ptr == input.data() + input.size()) {
            if (std::isfinite(result)) {
                return floating_point(result);
            }
        }

        return std::nullopt;
    }

    optional<floating_point> make_floating_point(double val)
    {
        if (std::isfinite(val)) {
            return floating_point(val);
        }
        return std::nullopt;
    }

    // Monoid operation: addition
    floating_point operator*(floating_point const& lhs, floating_point const& rhs)
    {
        return floating_point(lhs.value + rhs.value);
    }

    optional<floating_point> operator*(optional<floating_point> const& lhs, optional<floating_point> const& rhs)
    {
        if (!lhs || !rhs) return std::nullopt;
        return *lhs * *rhs;
    }

    floating_point operator+(floating_point const& lhs, floating_point const& rhs) { return lhs * rhs; }
    optional<floating_point> operator+(optional<floating_point> const& lhs, optional<floating_point> const& rhs) { return lhs * rhs; }

    floating_point operator-(floating_point const& lhs, floating_point const& rhs)
    {
        return floating_point(lhs.value - rhs.value);
    }

    // Comparison with epsilon
    constexpr double EPSILON = 1e-10;

    bool operator==(floating_point const& lhs, floating_point const& rhs)
    {
        return std::abs(lhs.value - rhs.value) < EPSILON;
    }
    bool operator!=(floating_point const& lhs, floating_point const& rhs) { return !(lhs == rhs); }
    bool operator<(floating_point const& lhs, floating_point const& rhs) { return lhs.value < rhs.value - EPSILON; }
    bool operator<=(floating_point const& lhs, floating_point const& rhs) { return lhs.value <= rhs.value + EPSILON; }
    bool operator>(floating_point const& lhs, floating_point const& rhs) { return lhs.value > rhs.value + EPSILON; }
    bool operator>=(floating_point const& lhs, floating_point const& rhs) { return lhs.value >= rhs.value - EPSILON; }

    std::ostream& operator<<(std::ostream& os, floating_point const& fp)
    {
        return os << fp.value;
    }

    floating_point operator|(floating_point const& lhs, floating_point const& rhs)
    {
        return lhs.empty() ? rhs : lhs;
    }

    optional<floating_point> operator|(optional<floating_point> const& lhs, optional<floating_point> const& rhs)
    {
        return lhs ? lhs : rhs;
    }

    floating_point operator^(floating_point const& base, size_t count)
    {
        return floating_point(base.value * static_cast<double>(count));
    }

    template<typename F>
        requires std::invocable<F, floating_point>
    auto operator%(floating_point const& value, F const& function) -> decltype(function(value))
    {
        return function(value);
    }

    template<typename F>
        requires std::invocable<F, floating_point>
    auto operator%(optional<floating_point> const& maybe_value, F const& function)
        -> optional<std::decay_t<decltype(function(*maybe_value))>>
    {
        if (!maybe_value) return std::nullopt;
        return function(*maybe_value);
    }

    // ============================================================================
    // scientific_notation: Exponential form numbers (e.g., 1.5e10)
    // ============================================================================

    /**
     * @brief Type-safe scientific notation with proper value semantics
     *
     * Handles formats like "1.5e10", "3.2E-5", etc.
     * Internally stores as double but preserves scientific format intent.
     */
    class scientific_notation
    {
    private:
        double value;

        explicit scientific_notation(double val) : value(val) {}

    public:
        friend optional<scientific_notation> make_scientific_notation(string_view input);
        friend optional<scientific_notation> make_scientific_notation(double val);

        // Monoid operations
        friend scientific_notation operator*(scientific_notation const& lhs, scientific_notation const& rhs);
        friend optional<scientific_notation> operator*(optional<scientific_notation> const& lhs, optional<scientific_notation> const& rhs);

        // Arithmetic operations
        friend scientific_notation operator+(scientific_notation const& lhs, scientific_notation const& rhs);
        friend optional<scientific_notation> operator+(optional<scientific_notation> const& lhs, optional<scientific_notation> const& rhs);

        // Extended operators
        friend scientific_notation operator|(scientific_notation const& lhs, scientific_notation const& rhs);
        friend scientific_notation operator^(scientific_notation const& base, size_t count);

        // Comparison operators
        friend bool operator==(scientific_notation const& lhs, scientific_notation const& rhs);
        friend bool operator!=(scientific_notation const& lhs, scientific_notation const& rhs);
        friend bool operator<(scientific_notation const& lhs, scientific_notation const& rhs);
        friend bool operator<=(scientific_notation const& lhs, scientific_notation const& rhs);
        friend bool operator>(scientific_notation const& lhs, scientific_notation const& rhs);
        friend bool operator>=(scientific_notation const& lhs, scientific_notation const& rhs);

        // Stream output
        friend std::ostream& operator<<(std::ostream& os, scientific_notation const& sn);

        // Default constructible
        scientific_notation() : value(0.0) {}

        // Full value semantics
        scientific_notation(scientific_notation const& other) = default;
        scientific_notation(scientific_notation&& other) = default;
        scientific_notation& operator=(scientific_notation const& other) = default;
        scientific_notation& operator=(scientific_notation&& other) = default;

        // Access interface
        double val() const { return value; }
        explicit operator double() const { return value; }
        string str() const;
        explicit operator string() const { return str(); }

        bool empty() const { return std::abs(value) < 1e-100; }
    };

    /**
     * @brief Validate and create scientific_notation from string input
     *
     * Accepts formats: [+-]?[0-9]*\.?[0-9]+[eE][+-]?[0-9]+
     */
    optional<scientific_notation> make_scientific_notation(string_view input)
    {
        if (input.empty()) {
            return std::nullopt;
        }

        // Must contain 'e' or 'E'
        auto e_pos = input.find_first_of("eE");
        if (e_pos == string_view::npos) {
            return std::nullopt;
        }

        // Parse using std::from_chars
        double result;
        auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), result);

        if (ec == std::errc() && ptr == input.data() + input.size()) {
            if (std::isfinite(result)) {
                return scientific_notation(result);
            }
        }

        return std::nullopt;
    }

    optional<scientific_notation> make_scientific_notation(double val)
    {
        if (std::isfinite(val)) {
            return scientific_notation(val);
        }
        return std::nullopt;
    }

    string scientific_notation::str() const
    {
        // Format in scientific notation
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.6e", value);
        return string(buffer);
    }

    // Monoid operation: addition
    scientific_notation operator*(scientific_notation const& lhs, scientific_notation const& rhs)
    {
        return scientific_notation(lhs.value + rhs.value);
    }

    optional<scientific_notation> operator*(optional<scientific_notation> const& lhs, optional<scientific_notation> const& rhs)
    {
        if (!lhs || !rhs) return std::nullopt;
        return *lhs * *rhs;
    }

    scientific_notation operator+(scientific_notation const& lhs, scientific_notation const& rhs) { return lhs * rhs; }
    optional<scientific_notation> operator+(optional<scientific_notation> const& lhs, optional<scientific_notation> const& rhs) { return lhs * rhs; }

    bool operator==(scientific_notation const& lhs, scientific_notation const& rhs)
    {
        return std::abs(lhs.value - rhs.value) < EPSILON * std::max(std::abs(lhs.value), std::abs(rhs.value));
    }
    bool operator!=(scientific_notation const& lhs, scientific_notation const& rhs) { return !(lhs == rhs); }
    bool operator<(scientific_notation const& lhs, scientific_notation const& rhs) { return lhs.value < rhs.value; }
    bool operator<=(scientific_notation const& lhs, scientific_notation const& rhs) { return lhs.value <= rhs.value; }
    bool operator>(scientific_notation const& lhs, scientific_notation const& rhs) { return lhs.value > rhs.value; }
    bool operator>=(scientific_notation const& lhs, scientific_notation const& rhs) { return lhs.value >= rhs.value; }

    std::ostream& operator<<(std::ostream& os, scientific_notation const& sn)
    {
        return os << sn.str();
    }

    scientific_notation operator|(scientific_notation const& lhs, scientific_notation const& rhs)
    {
        return lhs.empty() ? rhs : lhs;
    }

    optional<scientific_notation> operator|(optional<scientific_notation> const& lhs, optional<scientific_notation> const& rhs)
    {
        return lhs ? lhs : rhs;
    }

    scientific_notation operator^(scientific_notation const& base, size_t count)
    {
        return scientific_notation(base.value * static_cast<double>(count));
    }

    template<typename F>
        requires std::invocable<F, scientific_notation>
    auto operator%(scientific_notation const& value, F const& function) -> decltype(function(value))
    {
        return function(value);
    }

    template<typename F>
        requires std::invocable<F, scientific_notation>
    auto operator%(optional<scientific_notation> const& maybe_value, F const& function)
        -> optional<std::decay_t<decltype(function(*maybe_value))>>
    {
        if (!maybe_value) return std::nullopt;
        return function(*maybe_value);
    }
}

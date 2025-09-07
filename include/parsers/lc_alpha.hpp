#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <optional>
#include <iostream>
#include <vector>
#include <functional>

using std::string;
using std::string_view;
using std::optional;

namespace alga
{
    /**
     * @brief Type-safe lowercase alphabetic strings with proper value semantics
     * 
     * Forms a free monoid under concatenation with empty string as identity.
     * Now supports full value semantics (assignable, movable, storable in containers).
     */
    class lc_alpha
    {
    private:
        string s;  // Removed const to enable proper value semantics
        
        // Private constructor maintains invariant
        explicit lc_alpha(string str) : s(std::move(str)) {}
        
    public:
        // Factory function is the ONLY way to create from potentially invalid input
        friend optional<lc_alpha> make_lc_alpha(string_view input);
        
        // Monoid operations
        friend lc_alpha operator*(lc_alpha const& lhs, lc_alpha const& rhs);
        friend optional<lc_alpha> operator*(optional<lc_alpha> const& lhs, optional<lc_alpha> const& rhs);
        
        // Extended algebraic operators
        friend lc_alpha operator|(lc_alpha const& lhs, lc_alpha const& rhs);
        friend lc_alpha operator^(lc_alpha const& base, size_t count);
        
        // Comparison operators need access
        friend bool operator==(lc_alpha const& lhs, lc_alpha const& rhs);
        friend bool operator!=(lc_alpha const& lhs, lc_alpha const& rhs);
        friend bool operator<(lc_alpha const& lhs, lc_alpha const& rhs);
        friend bool operator<=(lc_alpha const& lhs, lc_alpha const& rhs);
        friend bool operator>(lc_alpha const& lhs, lc_alpha const& rhs);
        friend bool operator>=(lc_alpha const& lhs, lc_alpha const& rhs);
        
        // Default constructible (identity element)
        lc_alpha() = default;
        
        // Full value semantics enabled
        lc_alpha(lc_alpha const& other) = default;
        lc_alpha(lc_alpha&& other) = default;
        lc_alpha& operator=(lc_alpha const& other) = default;
        lc_alpha& operator=(lc_alpha&& other) = default;
        
        // Access interface
        string const& str() const { return s; }
        explicit operator string() const { return s; }
        
        // Iterator interface
        auto begin() const { return s.begin(); }
        auto end() const { return s.end(); }
        bool empty() const { return s.empty(); }
        size_t size() const { return s.size(); }
        
        // Direct access for internal operations (friend functions)
        string const& internal_string() const { return s; }
    };
    
    /**
     * @brief Validate and create lc_alpha from string input
     * 
     * This is the ONLY way to create lc_alpha from potentially invalid input.
     * Validates that input contains only alphabetic characters and converts to lowercase.
     * 
     * @param input String to validate and convert
     * @return optional<lc_alpha> containing the result if valid, nullopt if invalid
     */
    optional<lc_alpha> make_lc_alpha(string_view input)
    {
        // Check if all characters are alphabetic
        if (!std::all_of(input.begin(), input.end(), [](char c) { return std::isalpha(c); })) {
            return std::nullopt;
        }
        
        // Convert to lowercase
        string lowercase_str;
        lowercase_str.reserve(input.size());
        std::transform(input.begin(), input.end(), std::back_inserter(lowercase_str),
                      [](char c) { return std::tolower(c); });
        
        return lc_alpha(std::move(lowercase_str));
    }
    
    /**
     * @brief Direct composition for lc_alpha values
     * 
     * Implements the monoid operation directly on values for better compositionality.
     */
    lc_alpha operator*(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        string result = lhs.s + rhs.s;
        return lc_alpha(std::move(result));
    }

    /**
     * @brief Monadic composition for optional lc_alpha values
     * 
     * Implements the lifted monoid operation for optional values.
     * Returns nullopt if either operand is nullopt, otherwise concatenates.
     */
    optional<lc_alpha> operator*(optional<lc_alpha> const& lhs, optional<lc_alpha> const& rhs)
    {
        if (!lhs || !rhs) {
            return std::nullopt;
        }
        
        return *lhs * *rhs;
    }
    
    // Comparison operators
    bool operator==(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        return lhs.s == rhs.s;
    }
    
    bool operator!=(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        return !(lhs == rhs);
    }
    
    bool operator<(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        return lhs.s < rhs.s;
    }
    
    bool operator<=(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        return lhs.s <= rhs.s;
    }
    
    bool operator>(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        return lhs.s > rhs.s;
    }
    
    bool operator>=(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        return lhs.s >= rhs.s;
    }
    
    // Stream output operator
    std::ostream& operator<<(std::ostream& os, lc_alpha const& lc)
    {
        return os << lc.str();
    }

    // ============================================================================
    // Extended Algebraic Operators with C++20 Concepts
    // ============================================================================

    /**
     * @brief Choice operator - prefer left operand, fallback to right if left is empty
     */
    lc_alpha operator|(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        return lhs.empty() ? rhs : lhs;
    }

    /**
     * @brief Choice operator for optionals - first non-nullopt wins
     */
    optional<lc_alpha> operator|(optional<lc_alpha> const& lhs, optional<lc_alpha> const& rhs)
    {
        return lhs ? lhs : rhs;
    }

    /**
     * @brief Repetition operator - compose string with itself N times
     */
    lc_alpha operator^(lc_alpha const& base, size_t count)
    {
        if (count == 0) return lc_alpha{};
        if (count == 1) return base;
        
        string result;
        result.reserve(base.size() * count);
        for (size_t i = 0; i < count; ++i) {
            result += base.s;
        }
        return lc_alpha(std::move(result));
    }

    /**
     * @brief Sequential composition - creates ordered sequence
     */
    std::vector<lc_alpha> operator>>(lc_alpha const& lhs, lc_alpha const& rhs)
    {
        return {lhs, rhs};
    }

    /**
     * @brief Logical OR for optionals
     */
    optional<lc_alpha> operator||(optional<lc_alpha> const& lhs, optional<lc_alpha> const& rhs)
    {
        return lhs ? lhs : rhs;
    }

    /**
     * @brief Logical AND for optionals - both must succeed
     */
    optional<std::pair<lc_alpha, lc_alpha>> operator&&(optional<lc_alpha> const& lhs, optional<lc_alpha> const& rhs)
    {
        if (!lhs || !rhs) return std::nullopt;
        return std::make_pair(*lhs, *rhs);
    }

    /**
     * @brief Function application operator
     */
    template<typename F>
        requires std::invocable<F, lc_alpha>
    auto operator%(lc_alpha const& value, F const& function) -> decltype(function(value))
    {
        return function(value);
    }

    /**
     * @brief Function application for optional lc_alpha
     */
    template<typename F>
        requires std::invocable<F, lc_alpha>
    auto operator%(optional<lc_alpha> const& maybe_value, F const& function) 
        -> optional<std::decay_t<decltype(function(*maybe_value))>>
    {
        if (!maybe_value) return std::nullopt;
        return function(*maybe_value);
    }
}
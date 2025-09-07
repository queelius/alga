#pragma once

#include "lc_alpha.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <iostream>
#include <utility>
#include <vector>
#include <functional>

using std::string;
using std::optional;
using std::string_view;
using std::pair;

namespace alga
{
    // Forward declaration of stemming function
    void porter2stemmer(string &);

    /**
     * @brief Porter2 stemmed word with proper value semantics
     * 
     * Represents a word that has been processed by the Porter2 stemming algorithm.
     * Now supports full value semantics and uniform composition operations.
     */
    struct porter2_stem
    {
        lc_alpha word;  // Removed const to enable proper value semantics
        
        // Constructors
        porter2_stem() = default;
        porter2_stem(lc_alpha w) : word(std::move(w)) {}
        
        // Full value semantics
        porter2_stem(porter2_stem const& other) = default;
        porter2_stem(porter2_stem&& other) = default;
        porter2_stem& operator=(porter2_stem const& other) = default;
        porter2_stem& operator=(porter2_stem&& other) = default;
        
        // Access interface
        explicit operator string() const { return static_cast<string>(word); }
        lc_alpha const& lc() const { return word; }
        
        // Iterator interface  
        auto begin() const { return word.begin(); }
        auto end() const { return word.end(); }
        bool empty() const { return word.empty(); }
        size_t size() const { return word.size(); }
    };
    
    /**
     * @brief Factory function for porter2_stem from string input
     * 
     * Validates input, converts to lc_alpha, applies stemming, returns result.
     * This is the primary way to create porter2_stem from arbitrary string input.
     */
    optional<porter2_stem> make_porter2_stem(string_view input)
    {
        auto lc_opt = make_lc_alpha(input);
        if (!lc_opt) {
            return std::nullopt;
        }
        
        // Apply Porter2 stemming
        string s = lc_opt->str();
        porter2stemmer(s);
        
        auto stemmed_lc = make_lc_alpha(s);
        if (!stemmed_lc) {
            // This shouldn't happen if porter2stemmer is correct
            return std::nullopt;
        }
        
        return porter2_stem(std::move(*stemmed_lc));
    }
    
    /**
     * @brief Porter2 stemmer with uniform optional interface
     * 
     * Provides a clean, consistent interface where ALL operations return optional<T>.
     * No more mixed return types - everything follows the same pattern.
     */
    struct porter2_stemmer
    {
        using input_type = string_view;
        using output_type = porter2_stem;
        
        /**
         * @brief Stem a string input (primary interface)
         * 
         * @param input String to stem
         * @return optional<porter2_stem> Result if input is valid, nullopt if invalid
         */
        optional<porter2_stem> operator()(string_view input) const
        {
            return make_porter2_stem(input);
        }
        
        /**
         * @brief Stem an lc_alpha input 
         * 
         * @param input Valid lc_alpha to stem
         * @return optional<porter2_stem> Always succeeds for valid lc_alpha input
         */
        optional<porter2_stem> operator()(lc_alpha const& input) const
        {
            string s = input.str();
            porter2stemmer(s);
            
            auto stemmed_lc = make_lc_alpha(s);
            if (!stemmed_lc) {
                // This shouldn't happen if porter2stemmer is correct
                return std::nullopt;
            }
            
            return porter2_stem(std::move(*stemmed_lc));
        }
        
        /**
         * @brief Iterator-based parsing interface for composition
         * 
         * Parses alphabetic characters from iterator range and stems the result.
         * Returns pair of remaining iterator and optional result.
         */
        template<typename Iterator>
        pair<Iterator, optional<porter2_stem>> parse(Iterator begin, Iterator end) const
        {
            string word;
            Iterator current = begin;
            
            // Parse alphabetic characters
            while (current != end && std::isalpha(*current)) {
                word += std::tolower(*current);
                ++current;
            }
            
            if (word.empty()) {
                return {current, std::nullopt};
            }
            
            // Apply stemming
            porter2stemmer(word);
            
            auto lc_opt = make_lc_alpha(word);
            if (!lc_opt) {
                return {current, std::nullopt};
            }
            
            return {current, porter2_stem(std::move(*lc_opt))};
        }
    };
    
    /**
     * @brief Direct composition for porter2_stem values
     * 
     * Implements the monoid operation directly on values for better compositionality.
     */
    porter2_stem operator*(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        lc_alpha combined_word = lhs.word * rhs.word;
        return porter2_stem(combined_word);
    }

    /**
     * @brief Monadic composition for optional porter2_stem values
     * 
     * Implements uniform composition pattern across all algebraic types.
     */
    optional<porter2_stem> operator*(optional<porter2_stem> const& lhs, optional<porter2_stem> const& rhs)
    {
        if (!lhs || !rhs) {
            return std::nullopt;
        }
        
        return *lhs * *rhs;
    }
    
    // Comparison operators
    bool operator==(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        return lhs.word == rhs.word;
    }
    
    bool operator!=(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        return !(lhs == rhs);
    }
    
    bool operator<(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        return lhs.word < rhs.word;
    }
    
    bool operator<=(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        return lhs.word <= rhs.word;
    }
    
    bool operator>(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        return lhs.word > rhs.word;
    }
    
    bool operator>=(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        return lhs.word >= rhs.word;
    }
    
    // Stream output operator
    std::ostream& operator<<(std::ostream& os, porter2_stem const& stem)
    {
        return os << static_cast<string>(stem);
    }

    // ============================================================================
    // Extended Algebraic Operators with C++20 Concepts
    // ============================================================================

    /**
     * @brief Choice operator - prefer left operand, fallback to right if left is empty
     */
    porter2_stem operator|(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        return lhs.empty() ? rhs : lhs;
    }

    /**
     * @brief Choice operator for optionals - first non-nullopt wins
     */
    optional<porter2_stem> operator|(optional<porter2_stem> const& lhs, optional<porter2_stem> const& rhs)
    {
        return lhs ? lhs : rhs;
    }

    /**
     * @brief Repetition operator - compose stem with itself N times
     */
    porter2_stem operator^(porter2_stem const& base, size_t count)
    {
        if (count == 0) return porter2_stem{};
        if (count == 1) return base;
        
        porter2_stem result = base;
        for (size_t i = 1; i < count; ++i) {
            result = result * base;
        }
        return result;
    }

    /**
     * @brief Sequential composition - creates ordered sequence
     */
    std::vector<porter2_stem> operator>>(porter2_stem const& lhs, porter2_stem const& rhs)
    {
        return {lhs, rhs};
    }

    /**
     * @brief Logical OR for optionals
     */
    optional<porter2_stem> operator||(optional<porter2_stem> const& lhs, optional<porter2_stem> const& rhs)
    {
        return lhs ? lhs : rhs;
    }

    /**
     * @brief Logical AND for optionals - both must succeed
     */
    optional<std::pair<porter2_stem, porter2_stem>> operator&&(optional<porter2_stem> const& lhs, optional<porter2_stem> const& rhs)
    {
        if (!lhs || !rhs) return std::nullopt;
        return std::make_pair(*lhs, *rhs);
    }

    /**
     * @brief Function application operator
     */
    template<typename F>
        requires std::invocable<F, porter2_stem>
    auto operator%(porter2_stem const& value, F const& function) -> decltype(function(value))
    {
        return function(value);
    }

    /**
     * @brief Function application for optional porter2_stem
     */
    template<typename F>
        requires std::invocable<F, porter2_stem>
    auto operator%(optional<porter2_stem> const& maybe_value, F const& function) 
        -> optional<std::decay_t<decltype(function(*maybe_value))>>
    {
        if (!maybe_value) return std::nullopt;
        return function(*maybe_value);
    }
}
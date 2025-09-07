#pragma once

#include "porter2stemmer.hpp"
#include <array>
#include <algorithm>
#include <optional>
#include <iostream>

namespace alga
{
    /**
     * @brief N-gram of stemmed words with proper value semantics
     * 
     * Represents a fixed-size sequence of stemmed words that can be composed
     * and manipulated using algebraic operations. Now supports full value semantics.
     */
    template <int N, typename Stem>
    struct ngram_stem
    {
        std::array<Stem, N> stems;
        
        // Constructors
        ngram_stem() = default;
        
        // Initialize from array
        ngram_stem(std::array<Stem, N> s) : stems(std::move(s)) {}
        
        // Initialize from initializer list  
        template<typename... Args>
        ngram_stem(Args&&... args) : stems{std::forward<Args>(args)...} {
            static_assert(sizeof...(Args) == N, "Number of arguments must match ngram size");
        }
        
        // Full value semantics enabled by non-const members
        ngram_stem(ngram_stem const& other) = default;
        ngram_stem(ngram_stem&& other) = default;
        ngram_stem& operator=(ngram_stem const& other) = default;
        ngram_stem& operator=(ngram_stem&& other) = default;
        
        // Access interface
        constexpr size_t size() const { return N; }
        auto begin() const { return stems.begin(); }
        auto end() const { return stems.end(); }
        
        Stem const& operator[](size_t i) const { return stems[i]; }
        Stem& operator[](size_t i) { return stems[i]; }
        
        // String conversion
        explicit operator string() const
        {
            if constexpr (N == 0) {
                return "";
            } else if constexpr (N == 1) {
                return static_cast<string>(stems[0]);
            } else {
                string result;
                for (size_t i = 0; i < N; ++i) {
                    if (i > 0) result += " ";
                    result += static_cast<string>(stems[i]);
                }
                return result;
            }
        }
    };
    
    /**
     * @brief Factory function to create single-stem ngram
     */
    template<typename Stem>
    ngram_stem<1, Stem> make_ngram_stem(Stem stem)
    {
        return ngram_stem<1, Stem>{std::move(stem)};
    }
    
    /**
     * @brief Factory function to create ngram from multiple stems
     */
    template<typename... Args>
    auto make_ngram_stem(Args&&... args)
    {
        return ngram_stem<sizeof...(Args), std::decay_t<Args>...>{std::forward<Args>(args)...};
    }
    
    /**
     * @brief Composition of ngram_stems with clean template implementation
     * 
     * Now works perfectly with value semantics - no const member workarounds needed.
     */
    template <int N1, int N2, typename Stem>
    ngram_stem<N1 + N2, Stem> operator*(ngram_stem<N1, Stem> const& lhs, ngram_stem<N2, Stem> const& rhs)
    {
        std::array<Stem, N1 + N2> result;
        
        // Copy from left operand
        std::copy(lhs.stems.begin(), lhs.stems.end(), result.begin());
        
        // Copy from right operand  
        std::copy(rhs.stems.begin(), rhs.stems.end(), result.begin() + N1);
        
        return ngram_stem<N1 + N2, Stem>{std::move(result)};
    }
    
    /**
     * @brief Composition of single stem with ngram
     */
    template <int N, typename Stem>
    ngram_stem<N + 1, Stem> operator*(Stem const& lhs, ngram_stem<N, Stem> const& rhs)
    {
        std::array<Stem, N + 1> result;
        result[0] = lhs;
        std::copy(rhs.stems.begin(), rhs.stems.end(), result.begin() + 1);
        return ngram_stem<N + 1, Stem>{std::move(result)};
    }
    
    /**
     * @brief Composition of ngram with single stem
     */
    template <int N, typename Stem>  
    ngram_stem<N + 1, Stem> operator*(ngram_stem<N, Stem> const& lhs, Stem const& rhs)
    {
        std::array<Stem, N + 1> result;
        std::copy(lhs.stems.begin(), lhs.stems.end(), result.begin());
        result[N] = rhs;
        return ngram_stem<N + 1, Stem>{std::move(result)};
    }
    
    /**
     * @brief Monadic composition for optional ngram_stems
     */
    template <int N1, int N2, typename Stem>
    optional<ngram_stem<N1 + N2, Stem>> operator*(
        optional<ngram_stem<N1, Stem>> const& lhs,
        optional<ngram_stem<N2, Stem>> const& rhs)
    {
        if (!lhs || !rhs) {
            return std::nullopt;
        }
        return *lhs * *rhs;
    }
    
    /**
     * @brief Optional single stem * optional ngram composition
     */
    template <int N, typename Stem>
    optional<ngram_stem<N + 1, Stem>> operator*(
        optional<Stem> const& lhs,
        optional<ngram_stem<N, Stem>> const& rhs)
    {
        if (!lhs || !rhs) {
            return std::nullopt;
        }
        return *lhs * *rhs;
    }
    
    /**
     * @brief Optional ngram * optional single stem composition
     */
    template <int N, typename Stem>
    optional<ngram_stem<N + 1, Stem>> operator*(
        optional<ngram_stem<N, Stem>> const& lhs,
        optional<Stem> const& rhs)
    {
        if (!lhs || !rhs) {
            return std::nullopt;
        }
        return *lhs * *rhs;
    }
    
    // Comparison operators
    template <int N, typename Stem>
    bool operator==(ngram_stem<N, Stem> const& lhs, ngram_stem<N, Stem> const& rhs)
    {
        return lhs.stems == rhs.stems;
    }
    
    template <int N, typename Stem>
    bool operator!=(ngram_stem<N, Stem> const& lhs, ngram_stem<N, Stem> const& rhs)
    {
        return !(lhs == rhs);
    }
    
    template <int N, typename Stem>
    bool operator<(ngram_stem<N, Stem> const& lhs, ngram_stem<N, Stem> const& rhs)
    {
        return lhs.stems < rhs.stems;
    }
    
    // Stream output operator
    template <int N, typename Stem>
    std::ostream& operator<<(std::ostream& os, ngram_stem<N, Stem> const& ngram)
    {
        return os << static_cast<string>(ngram);
    }
    
    // Type aliases for common cases
    using unigram_stem = ngram_stem<1, porter2_stem>;
    using bigram_stem = ngram_stem<2, porter2_stem>;
    using trigram_stem = ngram_stem<3, porter2_stem>;
    
    // Convenience factory functions
    inline unigram_stem make_unigram(porter2_stem stem)
    {
        return unigram_stem{std::move(stem)};
    }
    
    inline bigram_stem make_bigram(porter2_stem stem1, porter2_stem stem2)
    {
        return bigram_stem{std::move(stem1), std::move(stem2)};
    }
    
    inline trigram_stem make_trigram(porter2_stem stem1, porter2_stem stem2, porter2_stem stem3)
    {
        return trigram_stem{std::move(stem1), std::move(stem2), std::move(stem3)};
    }
}
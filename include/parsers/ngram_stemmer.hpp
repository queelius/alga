
#pragma once

#include "porter2stemmer.hpp"
#include <array>

namespace algebraic_parsers
{
    template <int N, typename Stem>
    struct ngram_stem
    {
        std::array<Stem,N> stems;

        static constexpr auto sep() { return " "; }

        explicit operator string() const
        {
            string xs;
            for (auto x : stems)
                xs += sep() + x;
            return xs;
        }

        constexpr auto size() const { return N; }
        auto begin() const { return stems.begin(); }
        auto end() const { return stems.end(); }
    };

    auto ngram_stemmer(porter2_stem x)
    {
        return ngram_stem<1,porter2_stem>{ std::move(x) };
    }

    template <int N, typename Stem>
    auto ngram_stemmer(ngram_stem<N,Stem> x) { return x; }

    template <int N1, int N2, typename Stem>
    auto operator*(
        ngram_stem<N1,Stem> const & lhs,
        ngram_stem<N2,Stem> const & rhs)
    {
        std::array<Stem,N1+N2> stems(lhs.begin(),lhs.end());
        for (int i = 0; i < N2; ++i)
            stems[N1+i] = rhs[i];
        return ngram_stem
    }



    template <int N>
    auto operator*(
        porter2_stem const & lhs,
        ngram_stem<N,porter2_stem> const & rhs)
    {
        std::array<porter2_stem,N+1> stems;
        stems[0] = lhs;
        for (int i = 1; i < N+1; ++i)
            stems[i] = rhs.stems[i-1];
        return ngram_stem<N+1,porter2_stem>{ std::move(stems) };
    }


/*
    template <typename T>
    struct is_stem
    {
        static const bool value = false;
    };

    template <>
    struct is_stem<porter2_stem>
    {
        static const bool value = true;
    };

    template <typename T, typename... Ts>
    struct ngram_stem
    {

    };

    template <typename T, typename ... Ts>
    struct is_stem
    {
        static const bool value = is_stem<T>::value && is_stem<Ts...>::value;
    };
*/
}
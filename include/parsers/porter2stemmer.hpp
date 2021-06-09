#pragma once
/**
 * @file porter2stemmer.h
 * @author Sean Massung
 * @modified Alex Towell
 *
 * Implementation of
 * http://snowball.tartarus.org/algorithms/english/stemmer.html
 */

#include <array>
#include "lc_alpha.hpp"
#include <optional>
#include <string>
#include <utility>
using std::string;
using std::optional;
using std::pair;

namespace alex::text
{
    void porter2stemmer(string &);

    /**
     * porter2_stem has no algebraic structure. Any operation, with
     * the exception of concatenation with an empty string, may generate a
     * string that is not a porter2_stem.
     * 
     * We can *lift* the stemmer to a sequence of porter2_stems, however.
     * Let us call this lifting mechanism an ngram stem.
     */
    struct porter2_stem
    {
        using element_type = char;
        using value_type = lc_alpha;

        explicit operator string() const { return (string)w; }
        explicit operator lc_alpha() const { return w; }

        auto begin() const { return w.begin(); }
        auto end() const { return w.end(); }

        lc_alpha const w;
    };

    /**
     * Models the concept of a word stemmer, the porter2 stemmer, which
     * is a surjective function object, denoted porter2_stemmer, of type
     *     lc_alpha -> porter2_stem,
     * where lc_alpha is the set of lower-case alpha strings and porter2_stem
     * is a subset of lc_alpha consisting of the porter2 stems of lc_alpha
     * strings.
     * 
     * The stemmer also maps arbitrary strings, in which case it may be viewed
     * as a function of type
     *     string -> optional<porter2_stem>,
     * where any string not in lc_alpha is mapped to nullopt (nothing).
     * 
     * Strings with identical stems are in the same equivalence class, e.g., if
     *     porter2_stem("running") == porter2_stem("run")
     * then they are in the same class. This means that any string not in
     * lc_alpha is in the same equivalence class.
     * 
     * Observe that if porter2_stem(x) maps to nullopt, then x is not stemmable
     * by porter2_stem, which may used to compose stemmers or other types of
     * language grammars or parsers.
     */
    struct porter2_stemmer
    {
        using alphabet_type = char;
        using value_type = lc_alpha; // lower-case alphabetic subset of char*
        using output_type = porter2_stem;

        // I models a forward iterator such that value_type(I)
        // is convertible to string.
        /*template <typename I>
        pair<I,optional<porter2_stem>> operator()(I begin, I end) const
        {
            string x;
            while (begin != end && isalpha(*begin))
                x += (string)(*begin);
            return std::make_pair(begin,operator()(std::move(x)));
        }
        */

       static constexpr auto alphabet(alphbet_type c) { return isalpha(c); }

        auto operator()(lc_alpha const & x) const
        {
            auto s = (string)x;
            porter2stemmer(s);
            return porter2_stem{*make_lc_alpha(std::move(s))};
        }

        optional<porter2_stem> operator()(string x) const
        {
            auto w = make_lc_alpha(x);
            if (!w)
                return {};
            return operator()(*w);
        }
    };

    // Relational predicates.
    auto operator==(porter2_stem const & lhs, porter2_stem const & rhs)
    {
        return lhs.w == rhs.w;
    }

    auto operator!=(porter2_stem const & lhs, porter2_stem const & rhs)
    {
        return lhs.w != rhs.w;
    }

    auto operator>=(porter2_stem const & lhs, porter2_stem const & rhs)
    {
        return lhs.w >= rhs.w;
    }

    auto operator<=(porter2_stem const & lhs, porter2_stem const & rhs)
    {
        return lhs.w <= rhs.w;
    }

    auto operator>(porter2_stem const & lhs, porter2_stem const & rhs)
    {
        return lhs.w > rhs.w;
    }

    auto operator<(porter2_stem const & lhs, porter2_stem const & rhs)
    {
        return lhs.w < rhs.w;
    }
}

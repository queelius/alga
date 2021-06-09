#pragma once
/**
 * @file porter2stemmer.h
 * @author Sean Massung
 * @modified Alex Towell
 *
 * Implementation of
 * http://snowball.tartarus.org/algorithms/english/stemmer.html
 */

#include <string>
using std::string;

namespace alex::text
{

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
        using value_type = lc_alpha;

        porter2_stem(porter2_stem const &) = default;
        operator string() const { return (string)w; };
        operator lc_alpha() const { return w; };

        auto begin() const { return w.begin(); }
        auto end() const { return w.end(); }

        lc_alpha w;
    };


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


    auto concat()

    /**
     * Models the concept of a word stemmer, the porter2 stemmer, of type
     *     lc_alpha -> porter2_stem.
     * It accepts lowercase alpha strings as input, denoted by lc_alpha, and
     * outputs the porter2 stem, denoted by porter2_stem.
     * 
     * It also accepts strings, but if the string is not a lower-case alpha
     * string, it returns nothing,
     *     string -> optional<porter2_stem>.
     * 
     * Strings with identical stems are in the same equivalence class, e.g., if
     *     porter2_stem("running") == porter2_stem("run")
     * then they are in the same class.
     */
    struct porter2_stemmer
    {
        using input_type = lc_alpha;
        using output_type = porter2_stem;

        porter2_stem operator()(lc_alpha const & x)
        {
            auto s = (string)x;
            porter2stemmer(s);
            return porter2_stem(move(s));
        }

        optional<porter2_stem> operator()(string x)
        {
            auto w = to_lc_alpha(x);
            if (!w)
                return {};            
            
            porter2stemmer(x);            
            return porter2_stem(move(x));
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

    /**
     * A chain parser models a paser that has no branching. Either a T* 
     * In a chain parser that transforms a sequence of T to a sequence of U,
     *     T* -> U*
     * we could combine a stemmer
     *     stemmer : T -> optional<Stem>
     * equipped with
     *     convert : Stem -> U
     * with some transformer
     *     t : T -> U
     * by, for each element x of type T in T*, applying
     *     stemmer(x)
     * and, if successful, pushing that onto the output sequence U* otherwise
     * applying the other transformer
     *     t(x).
     * Note that t may also be a composition, and so this is a recursive
     * application with a base case that decides how to deal with unrecognized
     * strings. By default, this base case may be something simple, like
     * lower-case.
     * 
     *     
     * where Stem is equipped
     *      *     and some other processor to map sequences
     * of characters to a sequence of T, then the parser may first transform
     * the sequence of characters to a sequence of strings with whitespace and
     * other categories, like punctuation, removed, and then pass these strings
     * first to the porter2_stem and, if successful, map that to T (say, string)
     * and otherwise apply the other processor 
     */


#include <utility>
#include <optional>
using std::pair;
using std::optional;

template <typename P>
struct parser
{
    using token_parser = P;
    
    // I models a forward iterator such that value_type(I)
    // is convertible to string.
    template <typename I>
    pair<I,optional<P>> operator()(I begin, I end) const
    {
        string x;
        while (begin != end && isalpha(*begin))
            x += (string)(*begin);
        return std::make_pair(begin,operator()(std::move(x)));
    }
}


template <typename F, typename S, typename U>
struct exclusive_or_parser
{
    F f;
    S s;

    template <typename I>
    auto operator()(I begin, I end) const
    {
        std::vector<U> seq;
        while (begin != end)
        {
            auto res1 = f(begin,end);
            if (res.second)
            {
                seq.push_back(U(*res1));
                begin = res1.first;
                continue;
            }

            auto res2 = s(begin,end);
            if (res2.second)
            {
                seq.push_back(U(*res2));
                begin = res2.first;
            }
            ++begin;
        }

        return seq;
    }
}
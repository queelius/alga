#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <optional>
using std::string;
using std::string_view;
using std::optional;

namespace alex::text
{
    class lc_alpha
    {
    public:
        friend optional<lc_alpha> make_lc_alpha(string);
        friend lc_alpha operator*(lc_alpha const &, lc_alpha const &);

        lc_alpha() = default;
        lc_alpha(lc_alpha const & w) : s(w.s) {}
        explicit operator string() const { return s; }

        auto begin() const { return s.begin(); }
        auto end() const { return s.end(); }
        auto empty() const { return s.empty(); }
        auto size() const { return s.size(); }

    private:
        lc_alpha(string s) : s(move(s)) {}

        // invariant: s is a lower-case alpha string.
        string const s;
    };

    /**
     * The set of lower-case alpha strings, denoted by lc_alpha, equipped with
     * the binary operation
     *     operator * : (lc_alpha,lc_alpha) -> lc_alpha
     * and an identity lc_alpha() (empty string) is a free-monoid.
     */
    lc_alpha operator*(lc_alpha const & lhs, lc_alpha const & rhs)
    {
        return lc_alpha(lhs.s+rhs.s);
    }

    optional<lc_alpha> operator*(
        optional<lc_alpha> const & lhs,
        optional<lc_alpha> const & rhs)
    {
        return (lhs && rhs) ? optional<lc_alpha>((*lhs)*(*rhs)) : std::nullopt;
    }


    auto is_alpha(string_view s)
    {
        return std::all_of(s.begin(),s.end(), isalpha);
    }

    optional<lc_alpha> make_lc_alpha(string x)
    {
        if (x.empty())
            return lc_alpha();

        if (!is_alpha(x))
            return {};
            
        std::for_each(x.begin(),x.end(), [](auto & c) { c = tolower(c); });
        return lc_alpha(std::move(x));
    }

    auto operator==(lc_alpha const & lhs, lc_alpha const & rhs)
    {

        return std::equal(lhs.begin(),lhs.end(),rhs.begin(),rhs.end());
    }

    auto operator!=(lc_alpha const & lhs, lc_alpha const & rhs)
    {
        return !operator==(lhs,rhs);
    }

    auto operator>=(lc_alpha const & lhs, lc_alpha const & rhs)
    {
        return std::equal(lhs.begin(),lhs.end(),rhs.begin(),rhs.end(),
            [](auto l, auto r) { return l >= r; });
    }

    auto operator<=(lc_alpha const & lhs, lc_alpha const & rhs)
    {
        return std::equal(lhs.begin(),lhs.end(),rhs.begin(),rhs.end(),
            [](auto l, auto r) { return l <= r; });
    }

    auto operator>(lc_alpha const & lhs, lc_alpha const & rhs)
    {
        return std::equal(lhs.begin(),lhs.end(),rhs.begin(),rhs.end(),
            [](auto l, auto r) { return l > r; });
    }

    auto operator<(lc_alpha const & lhs, lc_alpha const & rhs)
    {
        return std::equal(lhs.begin(),lhs.end(),rhs.begin(),rhs.end(),
            [](auto l, auto r) { return l < r; });

    }
}
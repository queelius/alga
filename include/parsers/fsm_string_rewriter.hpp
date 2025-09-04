#pragma once

#include <regex>
#include <memory>
#include <algorithm>

using std::vector;
using std::string;
using std::move;
using std::pair;
using std::regex;

/**
 * Models the concept of the term rewriter.
 * 
 * It implements the model as a simple ordered sequence of production rules,
 * which is a pattern-substitution pair. The pattern is any regular expression
 * and the substitution is a string that can refer to regex backreferences.
 * 
 * The algorithm works in the following way. The list of production rules are
 * applied in the given order to an input string. If a production rule is
 * triggered, it goes back to the beginning of the sequence and starts over. If
 * it reaches the end of the sequence, it returns the rewritten string.
 */
struct fsm_string_rewriter
{
    using value_type = string;

    // regular expression
    using pattern_type = string;

    // regular expression
    using substitution_type = string;

    // rule_type: pattern -> substitution
    using rule_type = pair<pattern_type,substitution_type>;

    using iterator = vector<rule_type>::iterator;
    using const_iterator = vector<rule_type>::const_iterator;

    fsm_string_rewriter() = default;
    fsm_string_rewriter(fsm_string_rewriter const &) = default;

    template <typename I>
    fsm_string_rewriter(I begin, I end) : rules(begin,end) {}

    vector<rule_type> rules;
    regex::flag_type flags;

    auto begin() const { return rules.begin(); }
    auto end() const { return rules.end(); }
    auto begin() { return rules.begin(); }
    auto end() { return rules.end(); }

    auto ignore_case() const
    {
        return (flags & std::regex_constants::icase) == std::regex_constants::icase;
    }

    auto push(rule_type r) { return rules.push_back(r); }

    auto push(pattern_type p, substitution_type s)
    {
        return rules.push_back(rule_type{p,s});
    }

    string operator()(string x, int const max_iterations = 0) const
    {
        bool changed;
        int iterations = 0;
        do
        {
            changed = false;
            for (auto const & rule : rules)
            {
                auto new_x = std::regex_replace(
                    x,
                    regex(rule.first),
                    rule.second);

                if (x != new_x)
                {
                    changed = true;
                    x = move(new_x);
                }
            }
            ++iterations;
        } while (changed && (max_iterations == 0 ||
                (iterations < max_iterations)));
        return x;
    }
};

auto concat(
    fsm_string_rewriter lhs,
    fsm_string_rewriter const & rhs)
{
    lhs.rules.insert(lhs.rules.end(),rhs.rules.begin(),rhs.rules.end());
    return lhs;
}
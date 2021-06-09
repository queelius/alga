#pragma once

#include <vector>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <utility>
#include <regex>
#include <memory>
#include <regex>

using std::string;
using std::istream;
using std::vector;

/**
 * A parser from T to U takes in sequence over T and returns a sequence over U,
 * where a sequence of type V is an iterable object with a value type V.
 * 
 * The parser conceptually models a transformation of type
 *     T* -> U*.
 * 
 * word_parser Models the concept of a parser from strings to a vector of
 * strings. It is a parametric type that accepts as a template paramater
 * a type that models the concept of a string rewriter. 
 */   

template <typename RW>
struct word_parser
{
    using value_type = string;

    static auto default_word_pattern()
    {
        return "<([A-Za-z_ ])+>|[A-Za-z]+";
    }

    word_parser(RW rewriter = RW(), string word_pattern = default_word_pattern()) :
        word_pattern(move(word_pattern)),
        rewriter(move(rewriter)) {}

    auto operator()(string x, int max_iterations = 0) const
    {
        x = rewriter(std::move(x),max_iterations);
        auto begin = std::regex_iterator<string::iterator>(x.begin(), x.end(), word_pattern);
        auto end = std::regex_iterator<string::iterator>();

        vector<string> words;
        for (auto i = begin; i != end; ++i)
            words.push_back(i->str());

        return words;
    }

    auto operator()(istream & s, int max_iterations = 0) const
    {
        std::ostringstream os;
        os << s.rdbuf();
        return operator()(os.str(), max_iterations);
    }

    RW rewriter;
    std::regex word_pattern;
};


/*
    auto tokenize(string const & message, bool ignore_punctuation)
    {
        vector<string> tokens;
        string token;
        for (size_t i = 0; i < message.size(); ++i)
        {
            char c = message[i];
            if (isalnum(c))
            {
                c = tolower(c);
                token += c;
            }
            else if (c == '\'')
                continue;
            else
            {
                if (!token.empty())
                    tokens.push_back(move(token));
                
                if (!ignore_punctuation)
                {
                    if (c == '.' || c == ',' || c == '!' || c == '?' || c == ';' || c == ':' || c == '-')
                        tokens.push_back(string() + c);
                }
            }
        }

        if (!token.empty())
            tokens.push_back(token);

        return tokens;
    }
*/
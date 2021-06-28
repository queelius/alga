#pragma once

#include <string>
#include <sstream>
#include <utility>
#include "exp.hpp"

namespace aperture::detail
{
    template <typename T>
    auto try_cast(std::string const & s)
    {
        std::stringstream ss(s);
        T x;
        return ((ss >> x).fail() || !(ss >> std::ws).eof()) ?
            std::optional<T>() : std::optional<T>(x);
    }

    template <typename I>
    auto tokenize(I begin, I end)
    {
        static auto const r = std::regex("\\w+(?:'\\w+)?|[^\\w\\s]");
        return std::make_pair(std::sregex_iterator(begin,end,r),std::sregex_iterator());
    }

    exp * atom(std::string const & tok)
    {
        if (auto result = try_cast<int>(tok); result) return new integer(*result);
        return new symbol(tok);
    }

    /**
     * I models an input iterator over match_results<I>
     */
    template <typename I>
    exp * read(I & begin, I end)
    {
        if (begin == end)
            return new unexpected("read", "EOF");

        auto tok = *begin++;
        if (tok.str() == "(")
        {
            auto s = new sexp;
            while (begin->str() != ")")
                s = prepend(read(begin,end),s);
            ++begin;
            return s;
        }

        if (tok.str() == ")")
        {
            std::cerr << "unexpected closing paren\n";
            std::terminate();
        }
        return atom(tok.str());
    }

}
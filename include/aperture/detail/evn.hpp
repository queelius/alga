#pragma once
#include "proc.hpp"
#include "unbounded.hpp"
#include "exp.hpp"
#include <string>

namespace aperture::detail
{
    struct env : exp
    {
        std::map<std::string, exp*> values;
        env * parent;

        unique_ptr<exp> clone() const
        {
            std::map<std::string, exp*> vs;
            for (auto const [k,e] : values)
                vs[k] = e->clone();
            return make_unique<env>(vs,parent->clone());
        }

        auto lookup(std::string const & x)
        {
            auto e = find(x);
            return e == nullptr ?
                unbounded(x) : e->values[x];
        }

        auto find(std::string const & x)
        {
            if (values.count(x) != 0)
                return this;

            return parent == nullptr ?
                nullptr : parent->find(x);
        }
    };

    auto sum_int(sexp * xs)
    {
            int result = 0;
            for (auto i : args->value)
                result += dynamic_cast<integer*>(i)->value;
            return new integer(result);
    }

    auto global_env()
    {
        auto e = new env();
        e->values["+"] = new proc(&sum_int)

        return e;
    }

    auto global_env = standard_env();
}
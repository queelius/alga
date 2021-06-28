#pragma once

#include <nlohmann/json.hpp>
#include <memory>

using nlohmann::json;
using std::unique_ptr;
using std::make_unique;

namespace aperture::detail
{
    /**
     * exp : an expression (AST) in aperture.
     */
    struct exp
    {
        virtual json            to_json() = 0;
        virtual unique_ptr<exp> from_json(exp *) = 0;
        virtual unique_ptr<exp> clone() const = 0;

        /**
         * An expression is closed if it has no free variables.
         */
        //virtual bool closed() const = 0;

        //virtual exp * let(sexp *) const = 0;
    };

    exp * eval(exp * x, env * e = global_env)
    {
        if (auto v = dynamic_cast<symbol*>(x); v)
            return e->lookup(v->value);

        if (auto i = dynamic_cast<primitive*>(x); i)
            return i;

        if (auto p = dynamic_cast<proc*>(x); p)
            return p;

        if (auto l = dynamic_cast<lambda*>(x); l)
            return l;

        if (auto s = dynamic_cast<sexp*>(x); s)
            return eval(apply(eval(car(s),eval(cdr(s)),e)));

        return nullptr;
    }

}
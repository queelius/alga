#pragma once

#include "sexp.hpp"
#include "env.hpp"

namespace aperture::detail
{
    struct lambda : exp
    {
        sexp * params;
        sexp * body;
        env * e;
    };

    exp * apply(lambda * lam, sexp * args)
    {
        return eval(lam->body, extend(lam->e, args));
    }

    template <typename O> // O specifies the order of params
                          // default is lexographic
    auto close(exp * body, env * e, O o = O())
    {
        auto params = e->free(); // find unbounded symbols (free variables)
        std::sort(free,o);

        return make_unique(
                   lambda(make_list(params.begin(),params.end()),
                   body,extend(params.begin(),params.end(),e)));

        return [free,e=env()](sexp * args)
        {
            for (auto f : free)
            {
                e[free->value] = args->head;
                args = args->tail;
            }
            return eval(aperture,e);
        }

        // suppose free := [a,b,c]
        // then, param 1 is a
        //       param 2 is b
        //       param 3 is c
        //
        // when we apply the lifted aperture A to
        // (x,y,z), symbol a in A
        // is replaced by x, b is replaced by y,
        // and c is replaced by z.
        // so, we just create an environment e
        // with e.lookup(a) = x, for instance,
        // and call eval(A,e), which produces
        // another expression.
    }
}

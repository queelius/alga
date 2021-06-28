#pragma once

#include "sexp.hpp"

namespace aperture::detail
{
    struct proc : exp
    {
        using proc_ptr = (*exp)(sexp*);

        proc(proc_ptr p) : p(p) {}

        proc_ptr p;
    };

    exp * apply(proc * f, sexp * args)
    {
        return f->p(args);
    }

    template <typename X, typename Y>
    struct proc_fn
    {
        proc_fn(proc * p) : p(p) {}
        Y operator()(X const & x) { return apply(p,new prim<X>(x))->as<Y>(); }

        proc * p;
    };

    template <typename X, typename Y>
    auto lift(proc * f)
    {
        return proc_fn(f);
    }

    template <typename X, typename Y>
    auto unlift(proc_fn<X,Y> f)
    {
        return f.p;
    }
}
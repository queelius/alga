#pragma once
#include "exp.hpp"

namespace lamb::detail
{
    struct sexp : exp
    {
        exp * head;
        sexp * tail;

        sexp(exp * head, sexp * tail = nullptr) : head(head), tail(tail) {}

        unique_ptr<exp> clone() const override
        {
            return make_unique<sexp>(head->clone(),tail->clone());
        }

        //operator json() const { return ... };

        sexp(json const & s)
        {
            
        }
    };

    auto cdr(sexp * e)
    {
        return e->tail;
    }

    auto car(sexp * e)
    {
        return e->head;
    }

    auto empty(sexp * e)
    {
        return e == nullptr;
    }

    auto length(sexp * e)
    {
        int l = 0;
        while (e)
            { ++l; e = e->tail; }
        return l;
    }

    auto cons(exp * lhs, sexp * rhs)
    {
        return make_unique<sexp>(lhs,rhs);
    }

    /**
     * precondition: !empty(s)
     * postcondition: pointer to last element of list is returned;
     */
    auto last(sexp * s)
    {
        while (s->tail)
           s = s->tail;
        return s;
    }

    auto push_back(exp * lhs, sexp * rhs)
    {
        auto s = make_unique<sexp>(lhs);
        last(rhs)->tail = s.get();
        return rhs;
    }

    auto append(sexp * lhs, sexp * rhs)
    {
        if (empty(lhs))
            return rhs;

        last(lhs)->tail = rhs;
        return e
    }

    auto to_json(sexp * l)
    {
        json result;
        result["sexp"] = nullptr;
        if (empty(l))
            return result;
            
        while (l->head)
        {
            result["sexp"].push_back(l->head->to_json());
            l = l->tail;
        }
        return result;
    }


}
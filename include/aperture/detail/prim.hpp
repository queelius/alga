#include "exp.hpp"

namespace aperture::detail
{
    struct prim : exp
    {
        template <typename T>
        prim(T x) : x(new value<T>(x)) {}

        unique_ptr<exp> clone() const
        {
            return x->clone();
        }

        struct interface
        {
            virtual void print(int d) const = 0;
            virtual prim * clone() const = 0;
        };

        template <typename T>
        struct value final : interface
        {
            void print(int d) const override
            {
                indent(d);
                std::cout << value << "\n";
            }

            interface * clone() const override
            {
                return new value<T>(x);
            }

            T x;
        };

        interface * x;
    };


    template <typename X>
    auto lift(prim * e)
    {
        auto result = dynamic_cast<value<T>*>(e->x);
        return result ?
            result : std::optional<X>();
    }

    template <typename X>
    auto unlift(X x)
    {
        
    }
}
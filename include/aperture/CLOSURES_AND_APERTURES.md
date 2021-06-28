# Closures and apertures

## Closure
When mathematicans look at a sheet of paper containing math symbols,
in their minds they have some notion about what the symbols mean.
In this case, their mental model is providing the binding, the
*closure*, for these symbols.

## Aperture
If the mathetican does not know what some of the symbols on the sheet of paper
mean, then these symbols are free variables, in which case the mathematical
expression does not have an unambiguous meaning.

We say that such an expression with free variables is an aperture.
An aperture is "an opening, hole, or gap."
When an expression has free variables (unbounded symbols), we
call this expression an aperture. This is the complent of a closure,
in which every symbol in the expression is grounded.

When you evaluate an expression with a closure, the result is a complete
specification of the program. When we evaluate an expression without a closure,
the result is an aperture, which is an incomplete program specification.

## Generic programming
Programs can work on expressions, including aperture expressions, so we may also
think of apertures as a program template "to be filled in" at some later point.

Apertures, thus, may be seen as a partial program whose complete
specification is a function of the free variables.

One way to bind a free variable in an aperture is to pass it an
additional environment that bounds it to something.
Such environments can either be explicitly passed around as
first-class citizens, or more likely, the current environment may
be extended with a "let" statement.

For instance, suppose E is an aperture expression with x and y
being free variables. Then,

    (let ((x 1) (y 2)) (E))

is a closure. If, instead, we close only x,

    (let ((x 1)) (E))

then the result is another aperture expression with a free variable y.



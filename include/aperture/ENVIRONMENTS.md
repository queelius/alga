# Environments

Environments bind free variables to some grounded reference.

When mathematicans look at a sheet of paper containing math symbols,
in their minds they have some notion about what the symbols mean.
In this case, their mental model is providing the binding, the
*closure*, for these symbols.

If they do not know what some of the symbols mean, then these symbols
remain free variables and the mathematical expressions may not
have a clear meaning, or whose meaning may depend upon other factors.

We say that such an expression with free variables is an aperture
(see APERTURE.md). The free variables can be grounded in a number of
ways, but until that binding point, we may think of the expression as
representing a more abstract or incomplete expression.

Of course, programs can work on expressions, including aperture,
expressions, so we may also think of apertures as a program
template "to be filled in" at some later point.


We can change the meaning of an expression, or program, by changing
the environment. For instance, an expression may have a meaningful
evaluation whether a particular set of symbols are grounded as
integers or matrices, and so can create program templates, or
generic programs, that work as long as the way we ground the symbols
models some concept, e.g., the mathematical objects may be any
sort of semigroup, or ring, or module, or whatever.


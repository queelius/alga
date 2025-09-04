#include "parsers/porter2stemmer.h"
#include <string>
#include <iostream>

int main()
{
    std::cout << algebraic_parsers::porter2_stemmer::stem("Person") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("PERSON") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("Persons") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("PERSONS") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("Persons'") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("PERSONS'") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("Person's") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("PERSON'S") << "\n";
    

    std::cout << algebraic_parsers::porter2_stemmer::stem("person") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("persons") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("persons'") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("person's") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("person's persons persons' person's") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("should've") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("shouldn't've") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("shouldntve") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("shouldn't") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("can't") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("couldn't") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("should not") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("run") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("\"running\"") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("running") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("runner") << "\n";
    std::cout << algebraic_parsers::porter2_stemmer::stem("\"runners\"") << "\n";
}

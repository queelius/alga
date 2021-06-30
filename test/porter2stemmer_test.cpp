


#include "porter2stemmer.h"
#include <string>
#include <iostream>

int main()
{
    std::cout << alex::nlp::porter2stemmer::stem("Person") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("PERSON") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("Persons") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("PERSONS") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("Persons'") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("PERSONS'") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("Person's") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("PERSON'S") << "\n";
    

    std::cout << alex::nlp::porter2stemmer::stem("person") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("persons") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("persons'") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("person's") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("person's persons persons' person's") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("should've") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("shouldn't've") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("shouldntve") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("shouldn't") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("can't") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("couldn't") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("should not") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("run") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("\"running\"") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("running") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("runner") << "\n";
    std::cout << alex::nlp::porter2stemmer::stem("\"runners\"") << "\n";
}
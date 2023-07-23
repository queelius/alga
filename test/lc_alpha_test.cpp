#include "include/parsers/lc_alpha.hpp"
#include "include/parsers/porter2stemmer.hpp"
#include <iostream>

int main()
{
    auto x = alex::text::make_lc_alpha("TesTing");
    //std::cout << (std::string)(*x) << "\n";

    std::string s0 = "testinG";
    alex::text::porter2stemmer(s0);
    std::cout << s0 << "\n";

    auto s = (std::string)(*x);

    std::string s2 = "abc";
    std::string s3 = "abc123";
    std::string s4 = "abc123 abc";
    std::string s5 = "abc123  abc123";
    std::string s6 = "hello world";
    alex::text::porter2stemmer(s);

    alex::text::porter2stemmer(s2);
    alex::text::porter2stemmer(s3);
    alex::text::porter2stemmer(s4);
    alex::text::porter2stemmer(s5);
    alex::text::porter2stemmer(s6);

    std::cout << s << "\n";
    std::cout << s2 << "\n";
    std::cout << s3 << "\n";
    std::cout << s4 << "\n";
    std::cout << s5 << "\n";
    std::cout << s6 << "\n";

    auto s7 = alex::text::porter2_stemmer{}("TesTing");
    auto s8 = alex::text::porter2_stemmer{}("testering");
    std::cout << (std::string)(*s7) << "\n";
    std::cout << (std::string)(*s8) << "\n";
    std::cout << (*s7==*s8) << "\n";
}

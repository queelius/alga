#include <string>
#include <regex>
#include <iostream>
#include <sstream>
#include <optional>
#include <functional>
#include <map>










int main()
{
    //std::string s = "( 123 (aaa bbb) 456 abc )";
    std::string s = "(+ (+ 1 1 1 2 3 4) 2 1 2)";
    auto [b,e] = tokenize(s.begin(),s.end());

    auto result = read(b,e);
    result->print(0);

    auto c = eval(result);
    c->print(0);

}

#pragma once

#include <stack>

template <typename Relation>
class parser_arff
{
public:
    parser_arff(istream& ins): _ins(ins) {};

    void parse()
    {
        bypass_comments();
        get_relation_name();
        bypass_comments();
        get_attributes();
        bypass_comments();
        get_data();
    }

private:
    enum TOKEN
    {
        NOMINAL_RANGE_BEGIN,
        NUMBER,
        COMMA,
        NOMINAL_RANGE_END,
        CHARACTER,
        AT_SIGN,
        NOTHING,
        COMMENT,
        SPACE,
        LINE_BREAK
    };

    class token
    {
        TOKEN   _token;
        int     _index;
        char    _sym;
    };

    token           _next_token;
    int             _index;
    istream         _ins;
    Relation        _rel;
    char            _sym;

    stack<char> history;

    TOKEN peekNext()
    {
        return ins.peek();
    };

    void bypassComments()
    {
        getNext();
        while (nextToken == COMMENT)
        {
            while (nextToken != LINE_BREAK)
                getNext();

            getNext();
        }
    };

    bool match(TOKEN token)
    {
        getNext();
        return token == nextToken;
    };

    bool match(char symbol)
    {
        getNext();
        return this->symbol == symbol;
    };

    char get_symbol()
    {
        if (next_token == NOTHING)
            return 0;
        else
            return symbol;
    };

    void get_relation_name()
    {
        string s;

        get_next();
        s += get_symbol();

        while (true)
        {
            get_next();

            if (next_token == CHARACTER)
                s += get_symbol();
            else
                break;
        }

        to_lower(s);
        if (s != "@relation")
            throw "Error: expected @RELATION";

        s.clear();
        while (true)
        {
            get_next();
            switch (next_token)
            {
                case CHARACTER:
                    s += getSymbol();
                    break;
            }
        }

        if (s.empty())
            throw "Error: expected non-empty @RELATION identifier";

        rel.set_name(s);
    }

    void to_lower(string & s)
    {
        for (int i = 0; i < (int)s.length(); i++)
            tolower(s[i]);
    };

    void get_attributes()
    {
        // @attribute outlook {sunny, overcast, rainy}

        string s;
        ins >> s;
        to_lower(s);

        if (s != "relation")
            throw "Error: expected @relation";

        ins >> s;
        if (s.empty())
            throw "Error: expected non-empty string";

        rel.add_attribute(s);

        get_next();
        if (next_token != NOMINAL_RANGE_BEGIN)
            throw "Error: expected {";

        while (next_token != NOMINAL_RANGE_END)
        {
            get_nominal_range_element();
        }
    };

    void getData()
    {
    };

    void bypassComment()
    {

    };

    void putBack()
    {
        ins.putback(symbol);
    }

    void getNext() {
        ins.get(symbol);

        if (ins.eof())
            nextToken = NOTHING;
        else
        {
            history.push(symbol);
            switch (symbol) {
                case '{':
                    nextToken = NOMINAL_RANGE_BEGIN;
                    break;
                case '}':
                    nextToken = NOMINAL_RANGE_END;
                    break;
                case '\r':
                    if (ins.peek() == '\n')
                    {
                        ins.get(symbol);
                        nextToken = LINE_BREAK;
                    }
                case '\n':
                    nextToken = LINE_BREAK;
                    break;
                case '@':
                    nextToken = AT_SIGN;
                    break;
                case ',':
                    nextToken = COMMA;
                    break;
                case '%':
                    nextToken = COMMENT;
                    break;
                case ' ':
                    nextToken = SPACE;
                    break;
                default:
                    nextToken = CHARACTER;
                    break;
            }
        }
    };
};


#endif

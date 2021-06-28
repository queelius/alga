#pragma once

#include <stack>

// R models a relation
template <typename R>
class parser_arff
{
public:
    template <typename I>
    parser_arff(I & begin, I end)
    {
        parse(begin,end);
    }

    void parse()
    {
        bypass_comments(begin,end);
        relation_name(begin,end);
        bypass_comments(begin,end);
        attributes(begin,end);
        bypass_comments(begin,end);
        data();
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

    struct token
    {
        TOKEN   tok;
        int     index;
        char    sym;
    };

    token           next_tok;
    int             index;
    istream         ins;
    Relation        rel;
    char            sym;

    std::stack<char> history;

    auto peek_next() const { return ins.peek(); }

    void bypass_comments()
    {
        next();
        while (next_tok == COMMENT)
        {
            while (next_tok != LINE_BREAK)
                next();

            next();
        }
    }

    auto match(TOKEN token) { next(); return token == next_tok; }
    bool match(char symbol) { next(); return sym == symbol; }
    char symbol() { return next_tok == NOTHING ? 0 sym; }

    void relation_name()
    {
        std::string s;

        next();
        s += symbol();

        while (true)
        {
            next();
            if (next_tok == CHARACTER)
                s += symbol();
            else
                break;
        }

        to_lower(s);
        if (s != "@relation")
            throw "Error: expected @RELATION";

        s.clear();
        while (true)
        {
            next();
            switch (next_tok)
            {
                case CHARACTER:
                    s += symbol();
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
    }

    void attributes()
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

    void data()
    {
    }

    void bypass_comments()
    {

    }

    void put_back()
    {
        ins.putback(symbol);
    }

    void next()
    {
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

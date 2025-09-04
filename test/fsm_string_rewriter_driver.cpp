#include "parsers/word_parser.hpp"
#include "parsers/fsm_string_rewriter.hpp"
#include "parsers/ngram_stemmer.hpp"
#include <iostream>

auto production()
{
    fsm_string_rewriter writer;
    writer.push(
        "(https?:\\/\\/)?([\\da-z\\.-]+)\\.([a-z\\.]{2,6})([\\/\\w \\.-]*)*\\/?",
        "<URL>");
    writer.push(
        "[a-z0-9]+@[a-z0-9]+\\.[a-z]{2,4}",
        "<EMAIL>");
    writer.push(
        "([0-1]?[0-9]|[2][0-3]):([0-5][0-9])(am|pm)?",
        "<TIME>");
    writer.push(
        "\\$(\\d{1,3}(\\,\\d{3})*|(\\d+))(\\.\\d{2})?",
        "<CURRENCY>");
    writer.push(
        "(1[0-2]|0?[1-9])/(3[01]|[12][0-9]|0?[1-9])/(?:[0-9]{2})?[0-9]{2}",
        "<DATE>");
    writer.push(
        "\\b[-+]?[0-9]*\\.[0-9]+\\b",
        "<DECIMAL>");
    writer.push(
        "\\b[-+]?[0-9]+\\b",
        "<INTEGER>");
    writer.push(
        "----------+(\\s*",
        "<RULER>");
    writer.push(
        "==========+",
        "<RULER>");
    writer.push(
        "--+",
        "<DASHES>");
    writer.push(
        "(\\.\\s*)+",
        "<PERIOD>");
    writer.push(
        "(\\!\\s*)+",
        "<EXCLAMATION>");
    writer.push(
        "(\\:\\s*)+",
        "<COLON>");
    writer.push(
        "(\\;\\s*)+",
        "<SEMICOLON>");
    writer.push(
        "(\\?\\s*)+",
        "<QUESTION>");
    writer.push(
        "(,\\s*)+",
        "<COMMA>");
    writer.push(
        "((\\n\\r)(\\n\\r)+|\\n\\n+|\\r\\r+)",
        "<NEW_PARAGRAPH>");
    writer.push(
        "(\\\"|')",
        "");
    writer.push(
        "<EXCLAMATION> (<EXCLAMATION> ?)+",
        "<EXCITED>");
    writer.push(
        "<QUESTION> ( ?<QUESTION>)+",
        "<QUESTIONS>");
    writer.push(
        "(<CONFUSED> ?<EXCITED> ?)+",
        "<HYSTERIA>");
    writer.push(
        "(<DASHES> ?<DASHES> ?)+",
        "<DASHED_RULER>");
    writer.push(
        "(<DASHED_RULER> ?<RULER> ?)+",
        "<DASHED_RULER>");
    return writer;
}

int main()
{
    fsm_string_rewriter writer;
    writer.push(
        "\\.\\s*",
        "<period>");
    writer.push(
        "\\b([-+]?[0-9]+)\\b",
        "<integer>");
    writer.push(
        "<integer><period><integer>",
        "<decimal>");
    writer.push(
        "<period>( ?<period>)+",
        "<ellipses>");
    writer.push(
        "(<integer>)/(<integer>)", "<rational>");
    writer.push(
        "(<rational>|<decimal>|<integer>)\\s*((<rational>|<decimal>|<integer>)\\s*)+",
        "<number_sequence>");

    auto out = writer("  123/1222 12.2 1334... testing 123. 123 333 this is a test . 333");
    std::cout << out << "\n";

    word_parser<fsm_string_rewriter> parser(writer);
    auto ws = parser(out);

    for (auto w : ws)
        std::cout << "'" << w << "'\n";
}
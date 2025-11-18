/**
 * @file normalization_test.cpp
 * @brief Tests for text normalization utilities
 */

#include <gtest/gtest.h>
#include "parsers/normalization.hpp"
#include <string>

using namespace alga;
using namespace alga::normalization;

TEST(Normalization, ToLowercase) {
    EXPECT_EQ(to_lowercase("HELLO"), "hello");
    EXPECT_EQ(to_lowercase("Hello"), "hello");
    EXPECT_EQ(to_lowercase("hello"), "hello");
}

TEST(Normalization, ToUppercase) {
    EXPECT_EQ(to_uppercase("hello"), "HELLO");
    EXPECT_EQ(to_uppercase("Hello"), "HELLO");
    EXPECT_EQ(to_uppercase("HELLO"), "HELLO");
}

TEST(Normalization, TrimLeft) {
    EXPECT_EQ(trim_left("  hello"), "hello");
    EXPECT_EQ(trim_left("hello"), "hello");
    EXPECT_EQ(trim_left("\t\nhello"), "hello");
}

TEST(Normalization, TrimRight) {
    EXPECT_EQ(trim_right("hello  "), "hello");
    EXPECT_EQ(trim_right("hello"), "hello");
    EXPECT_EQ(trim_right("hello\t\n"), "hello");
}

TEST(Normalization, Trim) {
    EXPECT_EQ(trim("  hello  "), "hello");
    EXPECT_EQ(trim("\thello\n"), "hello");
    EXPECT_EQ(trim("hello"), "hello");
}

TEST(Normalization, NormalizeWhitespace) {
    EXPECT_EQ(normalize_whitespace("hello   world"), "hello world");
    EXPECT_EQ(normalize_whitespace("  hello  world  "), "hello world");
    EXPECT_EQ(normalize_whitespace("hello\t\nworld"), "hello world");
}

TEST(Normalization, RemoveWhitespace) {
    EXPECT_EQ(remove_whitespace("hello world"), "helloworld");
    EXPECT_EQ(remove_whitespace("  hello  "), "hello");
    EXPECT_EQ(remove_whitespace("a b c"), "abc");
}

TEST(Normalization, RemovePunctuation) {
    EXPECT_EQ(remove_punctuation("hello, world!"), "hello world");
    EXPECT_EQ(remove_punctuation("test-case"), "testcase");
}

TEST(Normalization, RemoveDigits) {
    EXPECT_EQ(remove_digits("hello123"), "hello");
    EXPECT_EQ(remove_digits("test1case2"), "testcase");
}

TEST(Normalization, KeepAlnum) {
    EXPECT_EQ(keep_alnum("hello, world! 123"), "helloworld123");
    EXPECT_EQ(keep_alnum("test@example.com"), "testexamplecom");
}

TEST(Normalization, KeepAlpha) {
    EXPECT_EQ(keep_alpha("hello123world"), "helloworld");
    EXPECT_EQ(keep_alpha("test@case"), "testcase");
}

TEST(Normalization, ReplaceChar) {
    EXPECT_EQ(replace_char("hello world", ' ', '_'), "hello_world");
    EXPECT_EQ(replace_char("aaa", 'a', 'b'), "bbb");
}

TEST(Normalization, ReplaceAll) {
    EXPECT_EQ(replace_all("hello hello", "hello", "hi"), "hi hi");
    EXPECT_EQ(replace_all("test", "es", "ES"), "tESt");
}

TEST(Normalization, NormalizeLineEndings) {
    EXPECT_EQ(normalize_line_endings("hello\r\nworld"), "hello\nworld");
    EXPECT_EQ(normalize_line_endings("test\rcase"), "test\ncase");
}

TEST(Normalization, CollapseRepeated) {
    EXPECT_EQ(collapse_repeated("heeello", 'e'), "hello");
    EXPECT_EQ(collapse_repeated("a--b--c", '-'), "a-b-c");
}

TEST(Normalization, NormalizeText) {
    EXPECT_EQ(normalize_text("  HELLO   WORLD  "), "hello world");
    EXPECT_EQ(normalize_text("Test\t\tCase"), "test case");
}

TEST(Normalization, ToSlug) {
    EXPECT_EQ(to_slug("Hello World!"), "hello-world");
    EXPECT_EQ(to_slug("Test Case 123"), "test-case-123");
    EXPECT_EQ(to_slug("  spaces  "), "spaces");
}

TEST(Normalization, ToTitleCase) {
    EXPECT_EQ(to_title_case("hello world"), "Hello World");
    EXPECT_EQ(to_title_case("HELLO WORLD"), "Hello World");
    EXPECT_EQ(to_title_case("test case"), "Test Case");
}

TEST(Normalization, EmptyStrings) {
    EXPECT_EQ(to_lowercase(""), "");
    EXPECT_EQ(trim(""), "");
    EXPECT_EQ(normalize_whitespace(""), "");
    EXPECT_EQ(to_slug(""), "");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

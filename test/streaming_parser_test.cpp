/**
 * @file streaming_parser_test.cpp
 * @brief Comprehensive tests for streaming parser support
 *
 * Tests BufferedStreamReader, line parsing, chunk parsing, and file parsing.
 */

#include <gtest/gtest.h>
#include "parsers/streaming_parser.hpp"
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <optional>

using namespace alga;
using namespace alga::streaming;

// ============================================================================
// Helper Parsers
// ============================================================================

class DigitParser {
public:
    using output_type = char;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<char>>
    {
        if (begin == end || !std::isdigit(*begin)) {
            return {begin, std::nullopt};
        }
        return {begin + 1, std::make_optional(*begin)};
    }
};

inline auto digit_parser() {
    return DigitParser{};
}

class IntParser {
public:
    using output_type = int;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<int>>
    {
        if (begin == end || !std::isdigit(*begin)) {
            return {begin, std::nullopt};
        }

        int result = 0;
        Iterator current = begin;

        while (current != end && std::isdigit(*current)) {
            result = result * 10 + (*current - '0');
            ++current;
        }

        return {current, std::make_optional(result)};
    }
};

inline auto int_parser() {
    return IntParser{};
}

class WordParser {
public:
    using output_type = std::string;

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        if (begin == end || !std::isalpha(*begin)) {
            return {begin, std::nullopt};
        }

        std::string result;
        Iterator current = begin;

        while (current != end && std::isalpha(*current)) {
            result += *current;
            ++current;
        }

        return {current, std::make_optional(std::move(result))};
    }
};

inline auto word_parser() {
    return WordParser{};
}

// ============================================================================
// BufferedStreamReader Tests
// ============================================================================

class BufferedStreamReaderTest : public ::testing::Test {};

TEST_F(BufferedStreamReaderTest, PeekWithoutConsuming) {
    std::istringstream input("hello");
    BufferedStreamReader reader(input);

    auto ch1 = reader.peek();
    ASSERT_TRUE(ch1.has_value());
    EXPECT_EQ(*ch1, 'h');

    auto ch2 = reader.peek();  // Should still be 'h'
    ASSERT_TRUE(ch2.has_value());
    EXPECT_EQ(*ch2, 'h');
}

TEST_F(BufferedStreamReaderTest, GetConsumes) {
    std::istringstream input("abc");
    BufferedStreamReader reader(input);

    EXPECT_EQ(*reader.get(), 'a');
    EXPECT_EQ(*reader.get(), 'b');
    EXPECT_EQ(*reader.get(), 'c');
    EXPECT_FALSE(reader.get().has_value());
}

TEST_F(BufferedStreamReaderTest, PeekAhead) {
    std::istringstream input("12345");
    BufferedStreamReader reader(input);

    EXPECT_EQ(*reader.peek_ahead(0), '1');
    EXPECT_EQ(*reader.peek_ahead(2), '3');
    EXPECT_EQ(*reader.peek_ahead(4), '5');
}

TEST_F(BufferedStreamReaderTest, ReadString) {
    std::istringstream input("hello world");
    BufferedStreamReader reader(input);

    std::string word = reader.read_string(5);
    EXPECT_EQ(word, "hello");

    reader.get();  // Skip space

    std::string word2 = reader.read_string(5);
    EXPECT_EQ(word2, "world");
}

TEST_F(BufferedStreamReaderTest, ReadWhile) {
    std::istringstream input("12345abc");
    BufferedStreamReader reader(input);

    std::string digits = reader.read_while([](char c) { return std::isdigit(c); });
    EXPECT_EQ(digits, "12345");

    std::string letters = reader.read_while([](char c) { return std::isalpha(c); });
    EXPECT_EQ(letters, "abc");
}

TEST_F(BufferedStreamReaderTest, AtEnd) {
    std::istringstream input("hi");
    BufferedStreamReader reader(input);

    EXPECT_FALSE(reader.at_end());
    reader.get();
    reader.get();
    EXPECT_TRUE(reader.at_end());
}

TEST_F(BufferedStreamReaderTest, EmptyStream) {
    std::istringstream input("");
    BufferedStreamReader reader(input);

    EXPECT_TRUE(reader.at_end());
    EXPECT_FALSE(reader.peek().has_value());
}

TEST_F(BufferedStreamReaderTest, LargeBuffer) {
    std::string large_input(10000, 'x');
    std::istringstream input(large_input);
    BufferedStreamReader reader(input, 4096);

    for (int i = 0; i < 10000; ++i) {
        auto ch = reader.get();
        ASSERT_TRUE(ch.has_value());
        EXPECT_EQ(*ch, 'x');
    }

    EXPECT_TRUE(reader.at_end());
}

// ============================================================================
// Line Parser Tests
// ============================================================================

class LineParserTest : public ::testing::Test {};

TEST_F(LineParserTest, ParseSingleLine) {
    std::istringstream input("hello");
    auto parser = by_line(word_parser());

    auto results = parser.parse_stream(input);

    ASSERT_EQ(results.size(), 1UL);
    ASSERT_TRUE(results[0].has_value());
    EXPECT_EQ(*results[0], "hello");
}

TEST_F(LineParserTest, ParseMultipleLines) {
    std::istringstream input("hello\nworld\ntest");
    auto parser = by_line(word_parser());

    auto results = parser.parse_stream(input);

    ASSERT_EQ(results.size(), 3UL);
    EXPECT_EQ(*results[0], "hello");
    EXPECT_EQ(*results[1], "world");
    EXPECT_EQ(*results[2], "test");
}

TEST_F(LineParserTest, FailedLineParse) {
    std::istringstream input("hello\n123\nworld");
    auto parser = by_line(word_parser());

    auto results = parser.parse_stream(input);

    ASSERT_EQ(results.size(), 3UL);
    EXPECT_TRUE(results[0].has_value());
    EXPECT_FALSE(results[1].has_value());  // "123" fails word parser
    EXPECT_TRUE(results[2].has_value());
}

TEST_F(LineParserTest, WithCallback) {
    std::istringstream input("123\n456\n789");
    auto parser = by_line(int_parser());

    std::vector<int> collected;
    parser.parse_with_callback(input, [&](size_t line_num, const std::string& line, auto result) {
        if (result) {
            collected.push_back(*result);
        }
    });

    ASSERT_EQ(collected.size(), 3UL);
    EXPECT_EQ(collected[0], 123);
    EXPECT_EQ(collected[1], 456);
    EXPECT_EQ(collected[2], 789);
}

TEST_F(LineParserTest, EmptyLines) {
    std::istringstream input("hello\n\nworld");
    auto parser = by_line(word_parser());

    auto results = parser.parse_stream(input);

    ASSERT_EQ(results.size(), 3UL);
    EXPECT_TRUE(results[0].has_value());
    EXPECT_FALSE(results[1].has_value());  // Empty line fails
    EXPECT_TRUE(results[2].has_value());
}

// ============================================================================
// Chunk Parser Tests
// ============================================================================

class ChunkParserTest : public ::testing::Test {};

TEST_F(ChunkParserTest, SingleChunk) {
    std::istringstream input("5");
    auto parser = by_chunks(digit_parser(), 10);

    auto results = parser.parse_stream(input);

    ASSERT_EQ(results.size(), 1UL);
    ASSERT_TRUE(results[0].has_value());
    EXPECT_EQ(*results[0], '5');
}

TEST_F(ChunkParserTest, MultipleChunks) {
    std::string data = "123456789";
    std::istringstream input(data);
    auto parser = by_chunks(digit_parser(), 3);  // 3 bytes per chunk

    auto results = parser.parse_stream(input);

    EXPECT_EQ(results.size(), 3UL);  // 9 bytes / 3 = 3 chunks
}

TEST_F(ChunkParserTest, WithCallback) {
    std::string data = "abcdefgh";
    std::istringstream input(data);
    auto parser = by_chunks(word_parser(), 4);

    size_t chunk_count = 0;
    parser.parse_with_callback(input, [&](size_t chunk_num, size_t bytes_read, auto result) {
        ++chunk_count;
        EXPECT_EQ(bytes_read, 4UL);
    });

    EXPECT_EQ(chunk_count, 2UL);  // 8 bytes / 4 = 2 chunks
}

// ============================================================================
// File Parser Tests
// ============================================================================

class FileParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test files
        {
            std::ofstream file("/tmp/alga_test_small.txt");
            file << "hello";
        }
        {
            std::ofstream file("/tmp/alga_test_lines.txt");
            file << "first\nsecond\nthird\n";
        }
        {
            std::ofstream file("/tmp/alga_test_numbers.txt");
            file << "123\n456\n789\n";
        }
    }

    void TearDown() override {
        std::remove("/tmp/alga_test_small.txt");
        std::remove("/tmp/alga_test_lines.txt");
        std::remove("/tmp/alga_test_numbers.txt");
    }
};

TEST_F(FileParserTest, ParseSmallFile) {
    auto parser = from_file("/tmp/alga_test_small.txt", word_parser());

    auto result = parser.parse();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST_F(FileParserTest, ParseNonexistentFile) {
    auto parser = from_file("/tmp/nonexistent.txt", word_parser());

    auto result = parser.parse();

    EXPECT_FALSE(result.has_value());
}

TEST_F(FileParserTest, ParseByLine) {
    auto parser = from_file("/tmp/alga_test_lines.txt", word_parser());

    std::vector<std::string> lines;
    bool success = parser.parse_by_line([&](size_t line_num, const std::string& line, auto result) {
        if (result) {
            lines.push_back(*result);
        }
    });

    EXPECT_TRUE(success);
    ASSERT_EQ(lines.size(), 3UL);
    EXPECT_EQ(lines[0], "first");
    EXPECT_EQ(lines[1], "second");
    EXPECT_EQ(lines[2], "third");
}

TEST_F(FileParserTest, ParseByLineWithNumbers) {
    auto parser = from_file("/tmp/alga_test_numbers.txt", int_parser());

    std::vector<int> numbers;
    parser.parse_by_line([&](size_t line_num, const std::string& line, auto result) {
        if (result) {
            numbers.push_back(*result);
        }
    });

    ASSERT_EQ(numbers.size(), 3UL);
    EXPECT_EQ(numbers[0], 123);
    EXPECT_EQ(numbers[1], 456);
    EXPECT_EQ(numbers[2], 789);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(StreamingIntegration, BufferedReaderWithLineParser) {
    std::istringstream input("hello\nworld\n");
    BufferedStreamReader reader(input);

    std::string line1 = reader.read_while([](char c) { return c != '\n'; });
    EXPECT_EQ(line1, "hello");

    reader.get();  // Skip newline

    std::string line2 = reader.read_while([](char c) { return c != '\n'; });
    EXPECT_EQ(line2, "world");
}

TEST(StreamingIntegration, MultipleParserTypes) {
    std::istringstream input("hello123");
    BufferedStreamReader reader(input);

    std::string word = reader.read_while([](char c) { return std::isalpha(c); });
    EXPECT_EQ(word, "hello");

    std::string num = reader.read_while([](char c) { return std::isdigit(c); });
    EXPECT_EQ(num, "123");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(StreamingEdgeCases, VeryLongLine) {
    std::string long_line(100000, 'a');
    std::istringstream input(long_line);

    auto parser = by_line(word_parser());
    auto results = parser.parse_stream(input);

    ASSERT_EQ(results.size(), 1UL);
    ASSERT_TRUE(results[0].has_value());
    EXPECT_EQ(results[0]->size(), 100000UL);
}

TEST(StreamingEdgeCases, ManyShortLines) {
    std::ostringstream oss;
    for (int i = 0; i < 1000; ++i) {
        oss << "x\n";
    }

    std::istringstream input(oss.str());
    auto parser = by_line(word_parser());
    auto results = parser.parse_stream(input);

    EXPECT_EQ(results.size(), 1000UL);
}

TEST(StreamingEdgeCases, EmptyStream) {
    std::istringstream input("");
    auto parser = by_line(word_parser());

    auto results = parser.parse_stream(input);

    EXPECT_TRUE(results.empty());
}

TEST(StreamingEdgeCases, OnlyNewlines) {
    std::istringstream input("\n\n\n");
    auto parser = by_line(word_parser());

    auto results = parser.parse_stream(input);

    EXPECT_EQ(results.size(), 3UL);
    for (const auto& result : results) {
        EXPECT_FALSE(result.has_value());
    }
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

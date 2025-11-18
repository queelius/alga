/**
 * @file parse_error_test.cpp
 * @brief Comprehensive tests for error reporting system
 *
 * Tests Position, Span, ParseError, ParseResult, and PositionTracker.
 */

#include <gtest/gtest.h>
#include "parsers/parse_error.hpp"
#include <string>
#include <sstream>

using namespace alga::error;

// ============================================================================
// Position Tests
// ============================================================================

TEST(PositionTest, DefaultConstruction) {
    Position pos;
    EXPECT_EQ(pos.line, 1UL);
    EXPECT_EQ(pos.column, 1UL);
    EXPECT_EQ(pos.offset, 0UL);
}

TEST(PositionTest, CustomConstruction) {
    Position pos(5, 12, 100);
    EXPECT_EQ(pos.line, 5UL);
    EXPECT_EQ(pos.column, 12UL);
    EXPECT_EQ(pos.offset, 100UL);
}

TEST(PositionTest, AdvanceRegularChar) {
    Position pos;
    pos.advance('a');

    EXPECT_EQ(pos.line, 1UL);
    EXPECT_EQ(pos.column, 2UL);
    EXPECT_EQ(pos.offset, 1UL);
}

TEST(PositionTest, AdvanceNewline) {
    Position pos;
    pos.advance('\n');

    EXPECT_EQ(pos.line, 2UL);
    EXPECT_EQ(pos.column, 1UL);
    EXPECT_EQ(pos.offset, 1UL);
}

TEST(PositionTest, AdvanceMultipleChars) {
    Position pos;
    std::string text = "hello\nworld";

    for (char c : text) {
        pos.advance(c);
    }

    EXPECT_EQ(pos.line, 2UL);
    EXPECT_EQ(pos.column, 6UL);  // "world" has 5 chars, so column is 6
    EXPECT_EQ(pos.offset, text.length());
}

TEST(PositionTest, ToString) {
    Position pos(10, 25, 500);
    std::string str = pos.to_string();

    EXPECT_NE(str.find("10"), std::string::npos);
    EXPECT_NE(str.find("25"), std::string::npos);
}

TEST(PositionTest, Comparison) {
    Position pos1(1, 1, 0);
    Position pos2(1, 1, 0);
    Position pos3(2, 1, 10);

    EXPECT_TRUE(pos1 == pos2);
    EXPECT_FALSE(pos1 == pos3);
    EXPECT_TRUE(pos1 != pos3);
    EXPECT_TRUE(pos1 < pos3);
}

// ============================================================================
// Span Tests
// ============================================================================

TEST(SpanTest, DefaultConstruction) {
    Span span;
    EXPECT_TRUE(span.empty());
}

TEST(SpanTest, CustomConstruction) {
    Position start(1, 1, 0);
    Position end(1, 10, 9);
    Span span(start, end);

    EXPECT_FALSE(span.empty());
    EXPECT_EQ(span.length(), 9UL);
}

TEST(SpanTest, SingleLineSpan) {
    Position start(5, 10, 100);
    Position end(5, 20, 110);
    Span span(start, end);

    std::string str = span.to_string();
    EXPECT_NE(str.find("line 5"), std::string::npos);
    EXPECT_NE(str.find("10"), std::string::npos);
    EXPECT_NE(str.find("20"), std::string::npos);
}

TEST(SpanTest, MultiLineSpan) {
    Position start(5, 10, 100);
    Position end(8, 5, 150);
    Span span(start, end);

    std::string str = span.to_string();
    EXPECT_NE(str.find("line 5"), std::string::npos);
    EXPECT_NE(str.find("line 8"), std::string::npos);
}

// ============================================================================
// Severity Tests
// ============================================================================

TEST(SeverityTest, ToString) {
    EXPECT_EQ(to_string(Severity::Error), "error");
    EXPECT_EQ(to_string(Severity::Warning), "warning");
    EXPECT_EQ(to_string(Severity::Info), "info");
}

// ============================================================================
// ParseError Tests
// ============================================================================

TEST(ParseErrorTest, BasicConstruction) {
    Position pos(5, 10, 100);
    ParseError err(pos, "test error");

    EXPECT_EQ(err.position.line, 5UL);
    EXPECT_EQ(err.severity, Severity::Error);
    EXPECT_EQ(err.message, "test error");
}

TEST(ParseErrorTest, WithSeverity) {
    Position pos(1, 1, 0);
    ParseError err(pos, Severity::Warning, "test warning");

    EXPECT_EQ(err.severity, Severity::Warning);
}

TEST(ParseErrorTest, ExpectSingle) {
    Position pos(1, 1, 0);
    ParseError err(pos, "parse failed");
    err.expect("digit");

    ASSERT_EQ(err.expected.size(), 1UL);
    EXPECT_EQ(err.expected[0], "digit");
}

TEST(ParseErrorTest, ExpectMultiple) {
    Position pos(1, 1, 0);
    ParseError err(pos, "parse failed");
    err.expect("digit").expect("letter");

    ASSERT_EQ(err.expected.size(), 2UL);
    EXPECT_EQ(err.expected[0], "digit");
    EXPECT_EQ(err.expected[1], "letter");
}

TEST(ParseErrorTest, ExpectVector) {
    Position pos(1, 1, 0);
    ParseError err(pos, "parse failed");
    err.expect(std::vector<std::string>{"a", "b", "c"});

    ASSERT_EQ(err.expected.size(), 3UL);
}

TEST(ParseErrorTest, ButFound) {
    Position pos(1, 1, 0);
    ParseError err(pos, "parse failed");
    err.but_found("!");

    ASSERT_TRUE(err.found.has_value());
    EXPECT_EQ(*err.found, "!");
}

TEST(ParseErrorTest, WithSpan) {
    Position pos(1, 1, 0);
    Span span(Position(1, 1, 0), Position(1, 5, 4));
    ParseError err(pos, "parse failed");
    err.with_span(span);

    ASSERT_TRUE(err.span.has_value());
    EXPECT_EQ(err.span->length(), 4UL);
}

TEST(ParseErrorTest, WithContext) {
    Position pos(1, 1, 0);
    ParseError err(pos, "parse failed");
    err.with_context("hello world");

    ASSERT_TRUE(err.context.has_value());
    EXPECT_EQ(*err.context, "hello world");
}

TEST(ParseErrorTest, Format) {
    Position pos(5, 12, 100);
    ParseError err(pos, "unexpected character");
    err.expect("digit").but_found("!");

    std::string formatted = err.format();

    EXPECT_NE(formatted.find("error"), std::string::npos);
    EXPECT_NE(formatted.find("line 5"), std::string::npos);
    EXPECT_NE(formatted.find("column 12"), std::string::npos);
    EXPECT_NE(formatted.find("digit"), std::string::npos);
    EXPECT_NE(formatted.find("!"), std::string::npos);
}

TEST(ParseErrorTest, ChainedBuilders) {
    Position pos(1, 1, 0);
    ParseError err = ParseError(pos, "parse failed")
        .expect("digit")
        .expect("letter")
        .but_found("!")
        .with_context("test context");

    EXPECT_EQ(err.expected.size(), 2UL);
    EXPECT_TRUE(err.found.has_value());
    EXPECT_TRUE(err.context.has_value());
}

// ============================================================================
// PositionTracker Tests
// ============================================================================

TEST(PositionTrackerTest, BasicConstruction) {
    std::string input = "hello";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    EXPECT_EQ(tracker.position().line, 1UL);
    EXPECT_EQ(tracker.position().column, 1UL);
    EXPECT_FALSE(tracker.at_end());
}

TEST(PositionTrackerTest, Advance) {
    std::string input = "hello";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    tracker.advance();
    EXPECT_EQ(tracker.position().column, 2UL);
    EXPECT_EQ(tracker.position().offset, 1UL);
}

TEST(PositionTrackerTest, AdvanceMultiple) {
    std::string input = "hello";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    tracker.advance(3);
    EXPECT_EQ(tracker.position().column, 4UL);
    EXPECT_EQ(tracker.position().offset, 3UL);
}

TEST(PositionTrackerTest, AdvanceWithNewline) {
    std::string input = "hello\nworld";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    tracker.advance(6);  // Advance to after newline
    EXPECT_EQ(tracker.position().line, 2UL);
    EXPECT_EQ(tracker.position().column, 1UL);
}

TEST(PositionTrackerTest, Peek) {
    std::string input = "hello";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    auto ch = tracker.peek();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(*ch, 'h');

    // Peek shouldn't advance position
    EXPECT_EQ(tracker.position().column, 1UL);
}

TEST(PositionTrackerTest, PeekAtEnd) {
    std::string input = "";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    auto ch = tracker.peek();
    EXPECT_FALSE(ch.has_value());
    EXPECT_TRUE(tracker.at_end());
}

TEST(PositionTrackerTest, Remaining) {
    std::string input = "hello world";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    tracker.advance(6);
    std::string_view remaining = tracker.remaining();

    EXPECT_EQ(remaining, "world");
}

TEST(PositionTrackerTest, GetContext) {
    std::string input = "the quick brown fox jumps over the lazy dog";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    tracker.advance(16);  // Position at 'f' in "fox"
    std::string context = tracker.get_context(10, 10);

    EXPECT_NE(context.find("brown"), std::string::npos);
    EXPECT_NE(context.find("fox"), std::string::npos);
}

TEST(PositionTrackerTest, SpanFrom) {
    std::string input = "hello world";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    Position start = tracker.position();
    tracker.advance(5);
    Span span = tracker.span_from(start);

    EXPECT_EQ(span.length(), 5UL);
    EXPECT_EQ(span.start.offset, 0UL);
    EXPECT_EQ(span.end.offset, 5UL);
}

// ============================================================================
// ParseResult Tests
// ============================================================================

TEST(ParseResultTest, SuccessConstruction) {
    ParseResult<int> result(42);

    EXPECT_TRUE(result.success());
    EXPECT_FALSE(result.failed());
    EXPECT_EQ(result.value(), 42);
}

TEST(ParseResultTest, FailureConstruction) {
    Position pos(1, 1, 0);
    ParseError err(pos, "test error");
    ParseResult<int> result(err);

    EXPECT_FALSE(result.success());
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.error().message, "test error");
}

TEST(ParseResultTest, SuccessFactory) {
    auto result = success<int>(100);

    EXPECT_TRUE(result.success());
    EXPECT_EQ(result.value(), 100);
}

TEST(ParseResultTest, FailureFactory) {
    Position pos(1, 1, 0);
    auto result = failure<int>(ParseError(pos, "fail"));

    EXPECT_TRUE(result.failed());
}

TEST(ParseResultTest, ValueOr) {
    auto success_result = success<int>(42);
    auto failure_result = failure<int>(ParseError(Position(), "fail"));

    EXPECT_EQ(success_result.value_or(100), 42);
    EXPECT_EQ(failure_result.value_or(100), 100);
}

TEST(ParseResultTest, ToOptional) {
    auto success_result = success<int>(42);
    auto failure_result = failure<int>(ParseError(Position(), "fail"));

    auto opt1 = success_result.to_optional();
    ASSERT_TRUE(opt1.has_value());
    EXPECT_EQ(*opt1, 42);

    auto opt2 = failure_result.to_optional();
    EXPECT_FALSE(opt2.has_value());
}

TEST(ParseResultTest, BoolConversion) {
    auto success_result = success<int>(42);
    auto failure_result = failure<int>(ParseError(Position(), "fail"));

    EXPECT_TRUE(static_cast<bool>(success_result));
    EXPECT_FALSE(static_cast<bool>(failure_result));
}

TEST(ParseResultTest, DereferenceOperator) {
    auto result = success<int>(42);

    EXPECT_EQ(*result, 42);
}

TEST(ParseResultTest, ArrowOperator) {
    struct TestStruct {
        int value = 99;
    };

    auto result = success<TestStruct>(TestStruct{});

    EXPECT_EQ(result->value, 99);
}

// ============================================================================
// Common Errors Tests
// ============================================================================

TEST(CommonErrorsTest, ExpectedChar) {
    Position pos(1, 5, 4);
    auto err = errors::expected_char(pos, 'a', 'b');

    EXPECT_EQ(err.position.column, 5UL);
    EXPECT_FALSE(err.expected.empty());
    EXPECT_TRUE(err.found.has_value());

    std::string formatted = err.format();
    EXPECT_NE(formatted.find("'a'"), std::string::npos);
    EXPECT_NE(formatted.find("'b'"), std::string::npos);
}

TEST(CommonErrorsTest, ExpectedOneOf) {
    Position pos(1, 1, 0);
    auto err = errors::expected_one_of(pos, "abc", 'x');

    EXPECT_EQ(err.expected.size(), 3UL);
    EXPECT_TRUE(err.found.has_value());
}

TEST(CommonErrorsTest, ExpectedEOF) {
    Position pos(1, 10, 9);
    auto err = errors::expected_eof(pos, 'x');

    EXPECT_FALSE(err.expected.empty());
    EXPECT_TRUE(err.found.has_value());
}

TEST(CommonErrorsTest, UnexpectedEOF) {
    Position pos(1, 20, 19);
    auto err = errors::unexpected_eof(pos, "closing brace");

    EXPECT_FALSE(err.expected.empty());
    EXPECT_TRUE(err.found.has_value());
}

TEST(CommonErrorsTest, InvalidFormat) {
    Position pos(1, 1, 0);
    auto err = errors::invalid_format(pos, "number", "contains letters");

    EXPECT_NE(err.message.find("invalid"), std::string::npos);
    EXPECT_NE(err.message.find("number"), std::string::npos);
}

TEST(CommonErrorsTest, Custom) {
    Position pos(5, 10, 100);
    auto err = errors::custom(pos, "custom error message");

    EXPECT_EQ(err.message, "custom error message");
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(IntegrationTest, FullErrorReport) {
    std::string input = "the quick brown fox";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    // Advance to 'b' in "brown"
    tracker.advance(10);

    Position error_pos = tracker.position();
    std::string context = tracker.get_context(5, 5);

    auto err = ParseError(error_pos, "unexpected character")
        .expect("digit")
        .but_found("b")  // Just "b", not "'b'"
        .with_context(context);

    std::string formatted = err.format();

    EXPECT_NE(formatted.find("line 1"), std::string::npos);
    EXPECT_NE(formatted.find("column 11"), std::string::npos);
    EXPECT_NE(formatted.find("digit"), std::string::npos);
    EXPECT_NE(formatted.find("b"), std::string::npos);  // Check for "b" not "'b'"
    EXPECT_NE(formatted.find("context"), std::string::npos);
}

TEST(IntegrationTest, MultilineTracking) {
    std::string input = "line 1\nline 2\nline 3";
    PositionTracker<std::string::iterator> tracker(input.begin(), input.end());

    // Track through the string character by character
    // "line 1\n" = 7 chars, puts us at line 2, column 1
    // "line 2\n" = 7 more chars, puts us at line 3, column 1
    for (size_t i = 0; i < 14; ++i) {  // "line 1\nline 2\n" is 14 chars
        tracker.advance();
    }

    Position pos = tracker.position();
    EXPECT_EQ(pos.line, 3UL);
    EXPECT_EQ(pos.column, 1UL);
    EXPECT_EQ(pos.offset, 14UL);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

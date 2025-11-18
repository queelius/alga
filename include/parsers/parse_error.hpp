#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <variant>
#include <sstream>
#include <iomanip>

namespace alga {
namespace error {

/**
 * @brief Position in source input (line, column, byte offset)
 *
 * Used for tracking where parse errors occur.
 * Lines and columns are 1-indexed for human readability.
 */
struct Position {
    size_t line;     // 1-indexed line number
    size_t column;   // 1-indexed column number
    size_t offset;   // 0-indexed byte offset from start

    Position() : line(1), column(1), offset(0) {}
    Position(size_t l, size_t c, size_t o) : line(l), column(c), offset(o) {}

    /**
     * @brief Advance position by one character
     */
    void advance(char c) {
        ++offset;
        if (c == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }

    /**
     * @brief Format position as string (e.g., "line 5, column 12")
     */
    std::string to_string() const {
        std::ostringstream oss;
        oss << "line " << line << ", column " << column;
        return oss.str();
    }

    bool operator==(Position const& other) const {
        return line == other.line && column == other.column && offset == other.offset;
    }

    bool operator!=(Position const& other) const {
        return !(*this == other);
    }

    bool operator<(Position const& other) const {
        return offset < other.offset;
    }
};

/**
 * @brief Span of source text (start and end positions)
 */
struct Span {
    Position start;
    Position end;

    Span() = default;
    Span(Position s, Position e) : start(s), end(e) {}

    /**
     * @brief Check if span is empty (start == end)
     */
    bool empty() const {
        return start == end;
    }

    /**
     * @brief Get length in bytes
     */
    size_t length() const {
        return end.offset - start.offset;
    }

    /**
     * @brief Format span as string
     */
    std::string to_string() const {
        if (start.line == end.line) {
            return "line " + std::to_string(start.line) +
                   ", columns " + std::to_string(start.column) +
                   "-" + std::to_string(end.column);
        } else {
            return start.to_string() + " to " + end.to_string();
        }
    }
};

/**
 * @brief Error severity levels
 */
enum class Severity {
    Error,      // Parse failure, cannot continue
    Warning,    // Potential issue, can continue
    Info        // Informational message
};

/**
 * @brief Convert severity to string
 */
inline std::string to_string(Severity sev) {
    switch (sev) {
        case Severity::Error:   return "error";
        case Severity::Warning: return "warning";
        case Severity::Info:    return "info";
        default:                return "unknown";
    }
}

/**
 * @brief Parse error information
 *
 * Contains all information about a parse failure:
 * - Where it occurred (position/span)
 * - What went wrong (message)
 * - What was expected vs found
 * - Context for debugging
 */
struct ParseError {
    Position position;              // Where the error occurred
    Severity severity;              // Error severity
    std::string message;            // Human-readable error message
    std::vector<std::string> expected; // What was expected (e.g., "digit", "identifier")
    std::optional<std::string> found;  // What was actually found
    std::optional<Span> span;       // Optional span of problematic text
    std::optional<std::string> context; // Optional surrounding context

    ParseError(Position pos, std::string msg)
        : position(pos), severity(Severity::Error), message(std::move(msg)) {}

    ParseError(Position pos, Severity sev, std::string msg)
        : position(pos), severity(sev), message(std::move(msg)) {}

    /**
     * @brief Add expected token/pattern
     */
    ParseError& expect(std::string expected_item) {
        expected.push_back(std::move(expected_item));
        return *this;
    }

    /**
     * @brief Add multiple expected items
     */
    ParseError& expect(std::vector<std::string> expected_items) {
        expected.insert(expected.end(),
                       std::make_move_iterator(expected_items.begin()),
                       std::make_move_iterator(expected_items.end()));
        return *this;
    }

    /**
     * @brief Set what was actually found
     */
    ParseError& but_found(std::string found_item) {
        found = std::move(found_item);
        return *this;
    }

    /**
     * @brief Set error span
     */
    ParseError& with_span(Span s) {
        span = s;
        return *this;
    }

    /**
     * @brief Set context string
     */
    ParseError& with_context(std::string ctx) {
        context = std::move(ctx);
        return *this;
    }

    /**
     * @brief Format error as human-readable string
     *
     * Example output:
     * error at line 5, column 12: unexpected character
     *   expected: digit, letter
     *   found: '!'
     */
    std::string format() const {
        std::ostringstream oss;

        // Header: severity and position
        oss << to_string(severity) << " at " << position.to_string() << ": " << message;

        // Expected items
        if (!expected.empty()) {
            oss << "\n  expected: ";
            for (size_t i = 0; i < expected.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << expected[i];
            }
        }

        // Found item
        if (found) {
            oss << "\n  found: " << *found;
        }

        // Span information
        if (span && !span->empty()) {
            oss << "\n  at " << span->to_string();
        }

        // Context
        if (context) {
            oss << "\n  context: " << *context;
        }

        return oss.str();
    }
};

/**
 * @brief Position tracker for input streams
 *
 * Tracks position as you iterate through input.
 * Use this with iterator-based parsers to maintain position.
 */
template<typename Iterator>
class PositionTracker {
private:
    Iterator start_;
    Iterator current_;
    Iterator end_;
    Position position_;

public:
    PositionTracker(Iterator start, Iterator end)
        : start_(start), current_(start), end_(end), position_() {}

    /**
     * @brief Get current position
     */
    Position position() const {
        return position_;
    }

    /**
     * @brief Get current iterator
     */
    Iterator current() const {
        return current_;
    }

    /**
     * @brief Check if at end
     */
    bool at_end() const {
        return current_ == end_;
    }

    /**
     * @brief Advance by one character
     */
    void advance() {
        if (current_ != end_) {
            position_.advance(*current_);
            ++current_;
        }
    }

    /**
     * @brief Advance by N characters
     */
    void advance(size_t n) {
        for (size_t i = 0; i < n && current_ != end_; ++i) {
            advance();
        }
    }

    /**
     * @brief Peek at current character without advancing
     */
    std::optional<char> peek() const {
        if (current_ != end_) {
            return *current_;
        }
        return std::nullopt;
    }

    /**
     * @brief Get remaining input as string_view
     */
    std::string_view remaining() const {
        return std::string_view(&*current_, std::distance(current_, end_));
    }

    /**
     * @brief Get context around current position (N chars before/after)
     */
    std::string get_context(size_t chars_before = 20, size_t chars_after = 20) const {
        // Find start of context
        Iterator ctx_start = current_;
        size_t count = 0;
        while (ctx_start != start_ && count < chars_before) {
            --ctx_start;
            ++count;
        }

        // Find end of context
        Iterator ctx_end = current_;
        count = 0;
        while (ctx_end != end_ && count < chars_after) {
            ++ctx_end;
            ++count;
        }

        std::string context(ctx_start, ctx_end);

        // Add marker at current position
        size_t marker_pos = std::distance(ctx_start, current_);
        if (marker_pos < context.size()) {
            context.insert(marker_pos, "⮜");  // Unicode left arrow
        }

        return context;
    }

    /**
     * @brief Create span from saved position to current
     */
    Span span_from(Position start_pos) const {
        return Span(start_pos, position_);
    }
};

/**
 * @brief Parse result - either success with value or failure with error
 *
 * This is an alternative to optional<T> that provides rich error information.
 */
template<typename T>
class ParseResult {
private:
    std::variant<T, ParseError> data_;

public:
    // Success constructor
    explicit ParseResult(T value) : data_(std::move(value)) {}

    // Failure constructor
    explicit ParseResult(ParseError error) : data_(std::move(error)) {}

    /**
     * @brief Check if parse succeeded
     */
    bool success() const {
        return std::holds_alternative<T>(data_);
    }

    /**
     * @brief Check if parse failed
     */
    bool failed() const {
        return std::holds_alternative<ParseError>(data_);
    }

    /**
     * @brief Get value (throws if error)
     */
    T& value() {
        return std::get<T>(data_);
    }

    T const& value() const {
        return std::get<T>(data_);
    }

    /**
     * @brief Get error (throws if success)
     */
    ParseError& error() {
        return std::get<ParseError>(data_);
    }

    ParseError const& error() const {
        return std::get<ParseError>(data_);
    }

    /**
     * @brief Get value or default
     */
    T value_or(T default_value) const {
        if (success()) {
            return value();
        }
        return default_value;
    }

    /**
     * @brief Convert to optional (loses error information)
     */
    std::optional<T> to_optional() const {
        if (success()) {
            return value();
        }
        return std::nullopt;
    }

    /**
     * @brief Implicit conversion to bool (success check)
     */
    explicit operator bool() const {
        return success();
    }

    /**
     * @brief Dereference operator (get value)
     */
    T& operator*() {
        return value();
    }

    T const& operator*() const {
        return value();
    }

    /**
     * @brief Arrow operator
     */
    T* operator->() {
        return &value();
    }

    T const* operator->() const {
        return &value();
    }
};

/**
 * @brief Factory function for success result
 */
template<typename T>
ParseResult<T> success(T value) {
    return ParseResult<T>(std::move(value));
}

/**
 * @brief Factory function for failure result
 */
template<typename T>
ParseResult<T> failure(ParseError error) {
    return ParseResult<T>(std::move(error));
}

/**
 * @brief Common error builders
 */
namespace errors {

/**
 * @brief Expected character error
 */
inline ParseError expected_char(Position pos, char expected, char found) {
    std::string exp_str(1, expected);
    std::string found_str(1, found);
    return ParseError(pos, "unexpected character")
        .expect("'" + exp_str + "'")
        .but_found("'" + found_str + "'");
}

/**
 * @brief Expected one of several characters
 */
inline ParseError expected_one_of(Position pos, std::string const& expected_chars, char found) {
    std::vector<std::string> expected;
    for (char c : expected_chars) {
        expected.push_back("'" + std::string(1, c) + "'");
    }
    return ParseError(pos, "unexpected character")
        .expect(expected)
        .but_found("'" + std::string(1, found) + "'");
}

/**
 * @brief Expected end of input
 */
inline ParseError expected_eof(Position pos, char found) {
    return ParseError(pos, "expected end of input")
        .expect("end of input")
        .but_found("'" + std::string(1, found) + "'");
}

/**
 * @brief Unexpected end of input
 */
inline ParseError unexpected_eof(Position pos, std::string expected) {
    return ParseError(pos, "unexpected end of input")
        .expect(expected)
        .but_found("end of input");
}

/**
 * @brief Invalid format error
 */
inline ParseError invalid_format(Position pos, std::string const& what, std::string const& details = "") {
    std::string msg = "invalid " + what;
    if (!details.empty()) {
        msg += ": " + details;
    }
    return ParseError(pos, msg);
}

/**
 * @brief Custom error with message
 */
inline ParseError custom(Position pos, std::string message) {
    return ParseError(pos, std::move(message));
}

} // namespace errors

} // namespace error
} // namespace alga

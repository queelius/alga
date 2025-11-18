#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdint>

using std::string;
using std::string_view;
using std::optional;

namespace alga {
namespace utf8 {

/**
 * @brief UTF-8 utilities and validation
 */

/**
 * @brief Get the length of a UTF-8 sequence from the first byte
 */
inline size_t utf8_sequence_length(unsigned char first_byte) {
    if ((first_byte & 0x80) == 0x00) return 1;  // 0xxxxxxx
    if ((first_byte & 0xE0) == 0xC0) return 2;  // 110xxxxx
    if ((first_byte & 0xF0) == 0xE0) return 3;  // 1110xxxx
    if ((first_byte & 0xF8) == 0xF0) return 4;  // 11110xxx
    return 0;  // Invalid
}

/**
 * @brief Validate a UTF-8 sequence
 */
inline bool is_valid_utf8_sequence(string_view sv) {
    if (sv.empty()) return false;

    size_t len = utf8_sequence_length(static_cast<unsigned char>(sv[0]));
    if (len == 0 || len > sv.size()) return false;

    // Check continuation bytes
    for (size_t i = 1; i < len; ++i) {
        if ((static_cast<unsigned char>(sv[i]) & 0xC0) != 0x80) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Decode a UTF-8 sequence to Unicode code point
 */
inline optional<uint32_t> decode_utf8(string_view sv) {
    if (!is_valid_utf8_sequence(sv)) {
        return std::nullopt;
    }

    size_t len = utf8_sequence_length(static_cast<unsigned char>(sv[0]));
    uint32_t codepoint = 0;

    switch (len) {
        case 1:
            codepoint = static_cast<unsigned char>(sv[0]);
            break;
        case 2:
            codepoint = ((static_cast<unsigned char>(sv[0]) & 0x1F) << 6) |
                       (static_cast<unsigned char>(sv[1]) & 0x3F);
            break;
        case 3:
            codepoint = ((static_cast<unsigned char>(sv[0]) & 0x0F) << 12) |
                       ((static_cast<unsigned char>(sv[1]) & 0x3F) << 6) |
                       (static_cast<unsigned char>(sv[2]) & 0x3F);
            break;
        case 4:
            codepoint = ((static_cast<unsigned char>(sv[0]) & 0x07) << 18) |
                       ((static_cast<unsigned char>(sv[1]) & 0x3F) << 12) |
                       ((static_cast<unsigned char>(sv[2]) & 0x3F) << 6) |
                       (static_cast<unsigned char>(sv[3]) & 0x3F);
            break;
    }

    return codepoint;
}

/**
 * @brief Encode a Unicode code point to UTF-8
 */
inline optional<string> encode_utf8(uint32_t codepoint) {
    string result;

    if (codepoint <= 0x7F) {
        // 1-byte sequence
        result += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
        // 2-byte sequence
        result += static_cast<char>(0xC0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        // 3-byte sequence
        result += static_cast<char>(0xE0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0x10FFFF) {
        // 4-byte sequence
        result += static_cast<char>(0xF0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        return std::nullopt;  // Invalid code point
    }

    return result;
}

/**
 * @brief Check if a Unicode code point is alphabetic
 *
 * Simplified check covering common Unicode ranges.
 * For production use, consider using ICU library for comprehensive support.
 */
inline bool is_unicode_alpha(uint32_t codepoint) {
    // ASCII alphabetic
    if ((codepoint >= 'A' && codepoint <= 'Z') ||
        (codepoint >= 'a' && codepoint <= 'z')) {
        return true;
    }

    // Latin-1 Supplement (À-ÿ)
    if (codepoint >= 0xC0 && codepoint <= 0xFF) {
        // Exclude multiplication and division signs
        return codepoint != 0xD7 && codepoint != 0xF7;
    }

    // Latin Extended-A (Ā-ſ)
    if (codepoint >= 0x0100 && codepoint <= 0x017F) {
        return true;
    }

    // Latin Extended-B (ƀ-ɏ)
    if (codepoint >= 0x0180 && codepoint <= 0x024F) {
        return true;
    }

    // Greek and Coptic (Ͱ-Ͽ, Α-ω)
    if (codepoint >= 0x0370 && codepoint <= 0x03FF) {
        return true;
    }

    // Cyrillic (Ѐ-ӿ)
    if (codepoint >= 0x0400 && codepoint <= 0x04FF) {
        return true;
    }

    // Hebrew (א-ת)
    if (codepoint >= 0x05D0 && codepoint <= 0x05EA) {
        return true;
    }

    // Arabic (ا-ي)
    if (codepoint >= 0x0621 && codepoint <= 0x064A) {
        return true;
    }

    // Hiragana (ぁ-ゔ)
    if (codepoint >= 0x3040 && codepoint <= 0x309F) {
        return true;
    }

    // Katakana (ァ-ヺ)
    if (codepoint >= 0x30A0 && codepoint <= 0x30FF) {
        return true;
    }

    // CJK Unified Ideographs (一-鿿)
    if (codepoint >= 0x4E00 && codepoint <= 0x9FFF) {
        return true;
    }

    return false;
}

/**
 * @brief Convert Unicode code point to lowercase
 *
 * Simplified conversion for common cases.
 */
inline uint32_t to_lowercase(uint32_t codepoint) {
    // ASCII uppercase
    if (codepoint >= 'A' && codepoint <= 'Z') {
        return codepoint + ('a' - 'A');
    }

    // Latin-1 uppercase (À-Þ, excluding × at 0xD7)
    if (codepoint >= 0xC0 && codepoint <= 0xDE && codepoint != 0xD7) {
        return codepoint + 0x20;
    }

    // Greek uppercase (Α-Ω)
    if (codepoint >= 0x0391 && codepoint <= 0x03A9) {
        return codepoint + 0x20;
    }

    // Cyrillic uppercase (А-Я)
    if (codepoint >= 0x0410 && codepoint <= 0x042F) {
        return codepoint + 0x20;
    }

    // For other scripts, return unchanged (proper handling requires ICU)
    return codepoint;
}

/**
 * @brief Validate that a string is valid UTF-8
 */
inline bool is_valid_utf8(string_view sv) {
    size_t i = 0;
    while (i < sv.size()) {
        size_t len = utf8_sequence_length(static_cast<unsigned char>(sv[i]));
        if (len == 0 || i + len > sv.size()) {
            return false;
        }

        if (!is_valid_utf8_sequence(sv.substr(i, len))) {
            return false;
        }

        i += len;
    }
    return true;
}

} // namespace utf8

/**
 * @brief Type-safe UTF-8 alphabetic strings with proper value semantics
 *
 * Similar to lc_alpha but handles Unicode/UTF-8 characters.
 * Forms a free monoid under concatenation with empty string as identity.
 */
class utf8_alpha
{
private:
    string s;

    explicit utf8_alpha(string str) : s(std::move(str)) {}

public:
    // Factory function is the ONLY way to create from potentially invalid input
    friend optional<utf8_alpha> make_utf8_alpha(string_view input);

    // Monoid operations
    friend utf8_alpha operator*(utf8_alpha const& lhs, utf8_alpha const& rhs);
    friend optional<utf8_alpha> operator*(optional<utf8_alpha> const& lhs, optional<utf8_alpha> const& rhs);

    // Extended algebraic operators
    friend utf8_alpha operator|(utf8_alpha const& lhs, utf8_alpha const& rhs);
    friend utf8_alpha operator^(utf8_alpha const& base, size_t count);

    // Comparison operators
    friend bool operator==(utf8_alpha const& lhs, utf8_alpha const& rhs);
    friend bool operator!=(utf8_alpha const& lhs, utf8_alpha const& rhs);
    friend bool operator<(utf8_alpha const& lhs, utf8_alpha const& rhs);
    friend bool operator<=(utf8_alpha const& lhs, utf8_alpha const& rhs);
    friend bool operator>(utf8_alpha const& lhs, utf8_alpha const& rhs);
    friend bool operator>=(utf8_alpha const& lhs, utf8_alpha const& rhs);

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, utf8_alpha const& ua);

    // Default constructible (identity element)
    utf8_alpha() = default;

    // Full value semantics
    utf8_alpha(utf8_alpha const& other) = default;
    utf8_alpha(utf8_alpha&& other) = default;
    utf8_alpha& operator=(utf8_alpha const& other) = default;
    utf8_alpha& operator=(utf8_alpha&& other) = default;

    // Access interface
    string const& str() const { return s; }
    explicit operator string() const { return s; }

    // Iterator interface
    auto begin() const { return s.begin(); }
    auto end() const { return s.end(); }
    bool empty() const { return s.empty(); }
    size_t size() const { return s.size(); }  // Byte size, not character count

    /**
     * @brief Get number of Unicode characters (not bytes)
     */
    size_t char_count() const {
        size_t count = 0;
        size_t i = 0;
        while (i < s.size()) {
            size_t len = utf8::utf8_sequence_length(static_cast<unsigned char>(s[i]));
            if (len == 0) break;  // Invalid UTF-8
            i += len;
            ++count;
        }
        return count;
    }

    /**
     * @brief Get vector of Unicode code points
     */
    std::vector<uint32_t> codepoints() const {
        std::vector<uint32_t> result;
        size_t i = 0;
        while (i < s.size()) {
            size_t len = utf8::utf8_sequence_length(static_cast<unsigned char>(s[i]));
            if (len == 0) break;

            auto cp = utf8::decode_utf8(string_view(s.data() + i, len));
            if (cp) {
                result.push_back(*cp);
            }
            i += len;
        }
        return result;
    }
};

/**
 * @brief Validate and create utf8_alpha from string input
 *
 * Validates that input is valid UTF-8 and contains only alphabetic characters.
 * Converts to lowercase.
 */
optional<utf8_alpha> make_utf8_alpha(string_view input)
{
    // First validate UTF-8
    if (!utf8::is_valid_utf8(input)) {
        return std::nullopt;
    }

    // Process each character
    string result;
    size_t i = 0;
    while (i < input.size()) {
        size_t len = utf8::utf8_sequence_length(static_cast<unsigned char>(input[i]));
        if (len == 0 || i + len > input.size()) {
            return std::nullopt;
        }

        auto codepoint = utf8::decode_utf8(input.substr(i, len));
        if (!codepoint) {
            return std::nullopt;
        }

        // Check if alphabetic
        if (!utf8::is_unicode_alpha(*codepoint)) {
            return std::nullopt;
        }

        // Convert to lowercase
        uint32_t lower = utf8::to_lowercase(*codepoint);
        auto encoded = utf8::encode_utf8(lower);
        if (!encoded) {
            return std::nullopt;
        }

        result += *encoded;
        i += len;
    }

    return utf8_alpha(std::move(result));
}

// Monoid operation: concatenation
utf8_alpha operator*(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return utf8_alpha(lhs.s + rhs.s);
}

optional<utf8_alpha> operator*(optional<utf8_alpha> const& lhs, optional<utf8_alpha> const& rhs)
{
    if (!lhs || !rhs) {
        return std::nullopt;
    }
    return *lhs * *rhs;
}

// Comparison operators
bool operator==(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return lhs.s == rhs.s;
}

bool operator!=(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return !(lhs == rhs);
}

bool operator<(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return lhs.s < rhs.s;
}

bool operator<=(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return lhs.s <= rhs.s;
}

bool operator>(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return lhs.s > rhs.s;
}

bool operator>=(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return lhs.s >= rhs.s;
}

// Stream output
std::ostream& operator<<(std::ostream& os, utf8_alpha const& ua)
{
    return os << ua.str();
}

// Choice operator
utf8_alpha operator|(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return lhs.empty() ? rhs : lhs;
}

optional<utf8_alpha> operator|(optional<utf8_alpha> const& lhs, optional<utf8_alpha> const& rhs)
{
    return lhs ? lhs : rhs;
}

// Repetition operator
utf8_alpha operator^(utf8_alpha const& base, size_t count)
{
    if (count == 0) return utf8_alpha{};
    if (count == 1) return base;

    string result;
    result.reserve(base.size() * count);
    for (size_t i = 0; i < count; ++i) {
        result += base.s;
    }
    return utf8_alpha(std::move(result));
}

// Sequential composition
std::vector<utf8_alpha> operator>>(utf8_alpha const& lhs, utf8_alpha const& rhs)
{
    return {lhs, rhs};
}

// Logical OR for optionals
optional<utf8_alpha> operator||(optional<utf8_alpha> const& lhs, optional<utf8_alpha> const& rhs)
{
    return lhs ? lhs : rhs;
}

// Logical AND for optionals
optional<std::pair<utf8_alpha, utf8_alpha>> operator&&(optional<utf8_alpha> const& lhs, optional<utf8_alpha> const& rhs)
{
    if (!lhs || !rhs) return std::nullopt;
    return std::make_pair(*lhs, *rhs);
}

// Function application operator
template<typename F>
    requires std::invocable<F, utf8_alpha>
auto operator%(utf8_alpha const& value, F const& function) -> decltype(function(value))
{
    return function(value);
}

template<typename F>
    requires std::invocable<F, utf8_alpha>
auto operator%(optional<utf8_alpha> const& maybe_value, F const& function)
    -> optional<std::decay_t<decltype(function(*maybe_value))>>
{
    if (!maybe_value) return std::nullopt;
    return function(*maybe_value);
}

} // namespace alga

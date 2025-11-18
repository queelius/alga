#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>

namespace alga {
namespace normalization {

/**
 * @brief Convert string to lowercase
 */
inline std::string to_lowercase(std::string_view s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        result += std::tolower(static_cast<unsigned char>(c));
    }
    return result;
}

/**
 * @brief Convert string to uppercase
 */
inline std::string to_uppercase(std::string_view s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        result += std::toupper(static_cast<unsigned char>(c));
    }
    return result;
}

/**
 * @brief Trim whitespace from start
 */
inline std::string trim_left(std::string_view s) {
    auto start = std::find_if_not(s.begin(), s.end(),
                                   [](unsigned char c) { return std::isspace(c); });
    return std::string(start, s.end());
}

/**
 * @brief Trim whitespace from end
 */
inline std::string trim_right(std::string_view s) {
    auto end = std::find_if_not(s.rbegin(), s.rend(),
                                 [](unsigned char c) { return std::isspace(c); }).base();
    return std::string(s.begin(), end);
}

/**
 * @brief Trim whitespace from both ends
 */
inline std::string trim(std::string_view s) {
    return trim_right(trim_left(s));
}

/**
 * @brief Normalize whitespace (collapse multiple spaces to single space)
 */
inline std::string normalize_whitespace(std::string_view s) {
    std::string result;
    result.reserve(s.size());

    bool prev_was_space = false;
    for (char c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!prev_was_space) {
                result += ' ';
                prev_was_space = true;
            }
        } else {
            result += c;
            prev_was_space = false;
        }
    }

    return trim(result);
}

/**
 * @brief Remove all whitespace
 */
inline std::string remove_whitespace(std::string_view s) {
    std::string result;
    result.reserve(s.size());

    for (char c : s) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            result += c;
        }
    }

    return result;
}

/**
 * @brief Remove punctuation
 */
inline std::string remove_punctuation(std::string_view s) {
    std::string result;
    result.reserve(s.size());

    for (char c : s) {
        if (!std::ispunct(static_cast<unsigned char>(c))) {
            result += c;
        }
    }

    return result;
}

/**
 * @brief Remove digits
 */
inline std::string remove_digits(std::string_view s) {
    std::string result;
    result.reserve(s.size());

    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            result += c;
        }
    }

    return result;
}

/**
 * @brief Keep only alphanumeric characters
 */
inline std::string keep_alnum(std::string_view s) {
    std::string result;
    result.reserve(s.size());

    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            result += c;
        }
    }

    return result;
}

/**
 * @brief Keep only alphabetic characters
 */
inline std::string keep_alpha(std::string_view s) {
    std::string result;
    result.reserve(s.size());

    for (char c : s) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            result += c;
        }
    }

    return result;
}

/**
 * @brief Replace all occurrences of a character
 */
inline std::string replace_char(std::string_view s, char from, char to) {
    std::string result(s);
    std::replace(result.begin(), result.end(), from, to);
    return result;
}

/**
 * @brief Replace all occurrences of a substring
 */
inline std::string replace_all(std::string_view s, std::string_view from, std::string_view to) {
    if (from.empty()) return std::string(s);

    std::string result;
    result.reserve(s.size());

    size_t pos = 0;
    while (pos < s.size()) {
        size_t found = s.find(from, pos);
        if (found == std::string_view::npos) {
            result.append(s.substr(pos));
            break;
        }

        result.append(s.substr(pos, found - pos));
        result.append(to);
        pos = found + from.size();
    }

    return result;
}

/**
 * @brief Normalize line endings to \n
 */
inline std::string normalize_line_endings(std::string_view s) {
    std::string result = replace_all(s, "\r\n", "\n");
    result = replace_all(result, "\r", "\n");
    return result;
}

/**
 * @brief Collapse multiple consecutive characters to single occurrence
 */
inline std::string collapse_repeated(std::string_view s, char c) {
    std::string result;
    result.reserve(s.size());

    char prev = '\0';
    for (char ch : s) {
        if (ch != c || prev != c) {
            result += ch;
        }
        prev = ch;
    }

    return result;
}

/**
 * @brief Comprehensive text normalization
 *
 * Applies multiple normalizations:
 * - Trim whitespace
 * - Normalize whitespace
 * - Convert to lowercase
 */
inline std::string normalize_text(std::string_view s) {
    return to_lowercase(normalize_whitespace(s));
}

/**
 * @brief Slug generation (URL-friendly string)
 *
 * Example: "Hello World!" -> "hello-world"
 */
inline std::string to_slug(std::string_view s) {
    std::string result = to_lowercase(s);

    // Replace spaces and punctuation with hyphens
    for (char& c : result) {
        if (std::isspace(static_cast<unsigned char>(c)) ||
            std::ispunct(static_cast<unsigned char>(c))) {
            c = '-';
        }
    }

    // Remove non-alphanumeric except hyphens
    std::string cleaned;
    for (char c : result) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-') {
            cleaned += c;
        }
    }

    // Collapse multiple hyphens
    result = collapse_repeated(cleaned, '-');

    // Trim hyphens from ends
    while (!result.empty() && result.front() == '-') {
        result.erase(result.begin());
    }
    while (!result.empty() && result.back() == '-') {
        result.pop_back();
    }

    return result;
}

/**
 * @brief Title case conversion
 *
 * Capitalizes first letter of each word.
 */
inline std::string to_title_case(std::string_view s) {
    std::string result;
    result.reserve(s.size());

    bool capitalize_next = true;
    for (char c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            result += c;
            capitalize_next = true;
        } else if (capitalize_next && std::isalpha(static_cast<unsigned char>(c))) {
            result += std::toupper(static_cast<unsigned char>(c));
            capitalize_next = false;
        } else {
            result += std::tolower(static_cast<unsigned char>(c));
        }
    }

    return result;
}

/**
 * @brief Remove accents/diacritics (ASCII-only approximation)
 *
 * Simple version - for full Unicode support, use utf8_alpha
 */
inline std::string remove_accents_simple(std::string_view s) {
    // This is a very simplified version
    // For proper Unicode handling, integrate with utf8_alpha
    std::string result;
    result.reserve(s.size());

    for (unsigned char c : s) {
        // Only handle basic Latin-1 supplement
        if (c >= 192 && c <= 197) result += 'A';       // À-Å
        else if (c == 199) result += 'C';              // Ç
        else if (c >= 200 && c <= 203) result += 'E';  // È-Ë
        else if (c >= 204 && c <= 207) result += 'I';  // Ì-Ï
        else if (c == 209) result += 'N';              // Ñ
        else if (c >= 210 && c <= 214) result += 'O';  // Ò-Ö
        else if (c >= 217 && c <= 220) result += 'U';  // Ù-Ü
        else if (c == 221) result += 'Y';              // Ý
        else if (c >= 224 && c <= 229) result += 'a';  // à-å
        else if (c == 231) result += 'c';              // ç
        else if (c >= 232 && c <= 235) result += 'e';  // è-ë
        else if (c >= 236 && c <= 239) result += 'i';  // ì-ï
        else if (c == 241) result += 'n';              // ñ
        else if (c >= 242 && c <= 246) result += 'o';  // ò-ö
        else if (c >= 249 && c <= 252) result += 'u';  // ù-ü
        else if (c == 253 || c == 255) result += 'y';  // ý, ÿ
        else result += c;
    }

    return result;
}

} // namespace normalization
} // namespace alga

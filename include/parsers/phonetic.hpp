#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <optional>

namespace alga {
namespace phonetic {

/**
 * @brief Soundex phonetic encoding
 *
 * Encodes words by sound, making similarly-sounding words produce
 * the same code. Format: Letter + 3 digits (e.g., "S530")
 *
 * Algorithm:
 * 1. Keep first letter
 * 2. Drop a,e,i,o,u,h,w,y
 * 3. Replace consonants with digits (b,f,p,v→1, c,g,j,k,q,s,x,z→2, etc.)
 * 4. Remove adjacent duplicates
 * 5. Pad/truncate to 4 characters
 */
class Soundex {
private:
    static char get_soundex_code(char c) {
        c = std::tolower(c);
        switch (c) {
            case 'b': case 'f': case 'p': case 'v':
                return '1';
            case 'c': case 'g': case 'j': case 'k':
            case 'q': case 's': case 'x': case 'z':
                return '2';
            case 'd': case 't':
                return '3';
            case 'l':
                return '4';
            case 'm': case 'n':
                return '5';
            case 'r':
                return '6';
            default:
                return '0';  // a,e,i,o,u,h,w,y
        }
    }

public:
    /**
     * @brief Encode a word using Soundex
     */
    static std::string encode(std::string_view word) {
        if (word.empty()) return "0000";

        std::string result;
        result.reserve(4);

        // Keep first letter (uppercase)
        result += std::toupper(word[0]);

        char prev_code = get_soundex_code(word[0]);

        // Process remaining letters
        for (size_t i = 1; i < word.size() && result.size() < 4; ++i) {
            char code = get_soundex_code(word[i]);

            // Skip vowels and duplicates
            if (code != '0' && code != prev_code) {
                result += code;
            }

            if (code != '0') {
                prev_code = code;
            }
        }

        // Pad with zeros to length 4
        while (result.size() < 4) {
            result += '0';
        }

        return result;
    }

    /**
     * @brief Check if two words sound alike (have same Soundex code)
     */
    static bool sounds_like(std::string_view word1, std::string_view word2) {
        return encode(word1) == encode(word2);
    }
};

/**
 * @brief Metaphone phonetic encoding
 *
 * More accurate than Soundex for English words.
 * Produces variable-length phonetic codes.
 *
 * Simplified implementation focusing on common patterns.
 */
class Metaphone {
private:
    static bool is_vowel(char c) {
        c = std::tolower(c);
        return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
    }

    static char at(std::string_view s, size_t i) {
        return i < s.size() ? std::tolower(s[i]) : '\0';
    }

public:
    /**
     * @brief Encode a word using Metaphone
     */
    static std::string encode(std::string_view word, size_t max_length = 4) {
        if (word.empty()) return "";

        std::string w;
        for (char c : word) {
            w += std::toupper(c);
        }

        std::string result;
        result.reserve(max_length);

        size_t i = 0;

        // Drop initial letters in certain situations
        if (w.size() >= 2) {
            if (w.substr(0, 2) == "PN" || w.substr(0, 2) == "KN" ||
                w.substr(0, 2) == "GN" || w.substr(0, 2) == "WR" ||
                w.substr(0, 2) == "AE") {
                i = 1;
            } else if (w[0] == 'X') {
                result += 'S';
                i = 1;
            } else if (w.substr(0, 2) == "WH") {
                result += 'W';
                i = 2;
            }
        }

        // Process each character
        while (i < w.size() && result.size() < max_length) {
            char c = w[i];
            char next = at(w, i + 1);
            char prev = i > 0 ? w[i - 1] : '\0';

            switch (c) {
                case 'A': case 'E': case 'I': case 'O': case 'U':
                    if (i == 0) result += c;
                    break;

                case 'B':
                    if (i == w.size() - 1 && prev == 'M') {
                        // Silent b in "dumb"
                    } else {
                        result += 'B';
                    }
                    break;

                case 'C':
                    if (next == 'H') {
                        result += 'X';
                        i++;
                    } else if (next == 'I' || next == 'E' || next == 'Y') {
                        result += 'S';
                    } else {
                        result += 'K';
                    }
                    break;

                case 'D':
                    if (next == 'G' && (at(w, i+2) == 'E' || at(w, i+2) == 'I' || at(w, i+2) == 'Y')) {
                        result += 'J';
                        i++;
                    } else {
                        result += 'T';
                    }
                    break;

                case 'G':
                    if (next == 'H' && !is_vowel(at(w, i+2))) {
                        // Silent gh
                    } else if (next == 'N' && i == w.size() - 2) {
                        // Silent g in "gn" at end
                    } else if (next == 'E' || next == 'I' || next == 'Y') {
                        result += 'J';
                    } else {
                        result += 'K';
                    }
                    break;

                case 'H':
                    if (!is_vowel(prev) || !is_vowel(next)) {
                        // H is silent unless between vowels
                    } else {
                        result += 'H';
                    }
                    break;

                case 'K':
                    if (prev != 'C') {
                        result += 'K';
                    }
                    break;

                case 'P':
                    if (next == 'H') {
                        result += 'F';
                        i++;
                    } else {
                        result += 'P';
                    }
                    break;

                case 'Q':
                    result += 'K';
                    break;

                case 'S':
                    if (next == 'H') {
                        result += 'X';
                        i++;
                    } else if (next == 'I' && (at(w, i+2) == 'O' || at(w, i+2) == 'A')) {
                        result += 'X';
                    } else {
                        result += 'S';
                    }
                    break;

                case 'T':
                    if (next == 'H') {
                        result += '0';
                        i++;
                    } else if (next == 'I' && (at(w, i+2) == 'O' || at(w, i+2) == 'A')) {
                        result += 'X';
                    } else {
                        result += 'T';
                    }
                    break;

                case 'V':
                    result += 'F';
                    break;

                case 'W': case 'Y':
                    if (is_vowel(next)) {
                        result += c;
                    }
                    break;

                case 'X':
                    result += "KS";
                    break;

                case 'Z':
                    result += 'S';
                    break;

                default:
                    result += c;
                    break;
            }

            i++;
        }

        return result;
    }

    /**
     * @brief Check if two words sound alike (have same Metaphone code)
     */
    static bool sounds_like(std::string_view word1, std::string_view word2, size_t max_length = 4) {
        return encode(word1, max_length) == encode(word2, max_length);
    }
};

/**
 * @brief Convenience functions
 */

inline std::string soundex(std::string_view word) {
    return Soundex::encode(word);
}

inline std::string metaphone(std::string_view word, size_t max_length = 4) {
    return Metaphone::encode(word, max_length);
}

inline bool sounds_like_soundex(std::string_view word1, std::string_view word2) {
    return Soundex::sounds_like(word1, word2);
}

inline bool sounds_like_metaphone(std::string_view word1, std::string_view word2, size_t max_length = 4) {
    return Metaphone::sounds_like(word1, word2, max_length);
}

} // namespace phonetic
} // namespace alga

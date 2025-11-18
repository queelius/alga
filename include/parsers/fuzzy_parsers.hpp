#pragma once

#include "parsers/similarity.hpp"
#include "parsers/phonetic.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <algorithm>

namespace alga {
namespace fuzzy {

/**
 * @brief Word parser for use with fuzzy matching
 */
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

/**
 * @brief Fuzzy word matcher - accepts words within edit distance
 *
 * Parses a word and accepts it if within max_distance of target.
 */
template<typename DistanceFunc>
class FuzzyWordMatcher {
public:
    using output_type = std::string;

private:
    std::string target;
    size_t max_distance;
    DistanceFunc distance_func;

public:
    FuzzyWordMatcher(std::string target_word, size_t max_dist, DistanceFunc func)
        : target(std::move(target_word)), max_distance(max_dist), distance_func(func) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        // First parse a word
        auto word_p = word_parser();
        auto [pos, word_result] = word_p.parse(begin, end);

        if (!word_result) {
            return {begin, std::nullopt};
        }

        // Check if within distance
        if (distance_func(*word_result, target) <= max_distance) {
            return {pos, std::move(word_result)};
        }

        return {begin, std::nullopt};
    }
};

/**
 * @brief Create a fuzzy matcher using Levenshtein distance
 */
inline auto fuzzy_match(std::string target, size_t max_distance = 2) {
    return FuzzyWordMatcher(
        std::move(target),
        max_distance,
        [](const std::string& a, const std::string& b) {
            return similarity::levenshtein_distance(a, b);
        }
    );
}

/**
 * @brief Create a fuzzy matcher using Damerau-Levenshtein distance
 *
 * Better for typos (handles transpositions).
 */
inline auto fuzzy_match_dl(std::string target, size_t max_distance = 2) {
    return FuzzyWordMatcher(
        std::move(target),
        max_distance,
        [](const std::string& a, const std::string& b) {
            return similarity::damerau_levenshtein_distance(a, b);
        }
    );
}

/**
 * @brief Phonetic matcher - accepts words that sound alike
 *
 * Uses Soundex algorithm.
 */
class PhoneticMatcher {
public:
    using output_type = std::string;

private:
    std::string target_code;
    std::string target;

public:
    explicit PhoneticMatcher(std::string target_word)
        : target_code(phonetic::soundex(target_word)), target(std::move(target_word)) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        auto word_p = word_parser();
        auto [pos, word_result] = word_p.parse(begin, end);

        if (!word_result) {
            return {begin, std::nullopt};
        }

        // Check if sounds like target
        if (phonetic::soundex(*word_result) == target_code) {
            return {pos, std::move(word_result)};
        }

        return {begin, std::nullopt};
    }
};

/**
 * @brief Create a phonetic matcher (Soundex)
 */
inline auto phonetic_match(std::string target) {
    return PhoneticMatcher(std::move(target));
}

/**
 * @brief Similarity matcher - accepts words above similarity threshold
 *
 * Uses Jaro-Winkler similarity.
 */
class SimilarityMatcher {
public:
    using output_type = std::string;

private:
    std::string target;
    double threshold;

public:
    SimilarityMatcher(std::string target_word, double thresh = 0.8)
        : target(std::move(target_word)), threshold(thresh) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        auto word_p = word_parser();
        auto [pos, word_result] = word_p.parse(begin, end);

        if (!word_result) {
            return {begin, std::nullopt};
        }

        // Check similarity
        double sim = similarity::jaro_winkler_similarity(*word_result, target);
        if (sim >= threshold) {
            return {pos, std::move(word_result)};
        }

        return {begin, std::nullopt};
    }
};

/**
 * @brief Create a similarity matcher
 */
inline auto similarity_match(std::string target, double threshold = 0.8) {
    return SimilarityMatcher(std::move(target), threshold);
}

/**
 * @brief Fuzzy choice - match closest to any candidate
 *
 * Tries to match any of the candidates, accepting closest within distance.
 */
template<typename DistanceFunc>
class FuzzyChoiceMatcher {
public:
    using output_type = std::string;

private:
    std::vector<std::string> candidates;
    size_t max_distance;
    DistanceFunc distance_func;

public:
    FuzzyChoiceMatcher(std::vector<std::string> cands, size_t max_dist, DistanceFunc func)
        : candidates(std::move(cands)), max_distance(max_dist), distance_func(func) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        auto word_p = word_parser();
        auto [pos, word_result] = word_p.parse(begin, end);

        if (!word_result) {
            return {begin, std::nullopt};
        }

        // Find closest candidate
        size_t min_distance = SIZE_MAX;
        std::string closest;

        for (const auto& candidate : candidates) {
            size_t dist = distance_func(*word_result, candidate);
            if (dist < min_distance) {
                min_distance = dist;
                closest = candidate;
            }
        }

        // Accept if within threshold
        if (min_distance <= max_distance) {
            return {pos, std::make_optional(closest)};  // Return normalized form
        }

        return {begin, std::nullopt};
    }
};

/**
 * @brief Create fuzzy choice matcher
 */
inline auto fuzzy_choice(std::vector<std::string> candidates, size_t max_distance = 2) {
    return FuzzyChoiceMatcher(
        std::move(candidates),
        max_distance,
        [](const std::string& a, const std::string& b) {
            return similarity::levenshtein_distance(a, b);
        }
    );
}

/**
 * @brief Case-insensitive matcher
 */
class CaseInsensitiveMatcher {
public:
    using output_type = std::string;

private:
    std::string target;

    static std::string to_lower(std::string_view s) {
        std::string result;
        result.reserve(s.size());
        for (char c : s) {
            result += std::tolower(c);
        }
        return result;
    }

public:
    explicit CaseInsensitiveMatcher(std::string target_word)
        : target(to_lower(target_word)) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        auto word_p = word_parser();
        auto [pos, word_result] = word_p.parse(begin, end);

        if (!word_result) {
            return {begin, std::nullopt};
        }

        if (to_lower(*word_result) == target) {
            return {pos, std::move(word_result)};
        }

        return {begin, std::nullopt};
    }
};

/**
 * @brief Create case-insensitive matcher
 */
inline auto case_insensitive(std::string target) {
    return CaseInsensitiveMatcher(std::move(target));
}

/**
 * @brief Combined fuzzy matcher - tries multiple strategies
 *
 * 1. Exact match
 * 2. Case-insensitive match
 * 3. Phonetic match
 * 4. Fuzzy match (edit distance)
 */
class CombinedFuzzyMatcher {
public:
    using output_type = std::string;

private:
    std::string target;
    std::string target_lower;
    std::string target_soundex;
    size_t max_distance;

    static std::string to_lower(std::string_view s) {
        std::string result;
        for (char c : s) result += std::tolower(c);
        return result;
    }

public:
    CombinedFuzzyMatcher(std::string target_word, size_t max_dist = 2)
        : target(target_word),
          target_lower(to_lower(target_word)),
          target_soundex(phonetic::soundex(target_word)),
          max_distance(max_dist) {}

    template<typename Iterator>
    auto parse(Iterator begin, Iterator end) const
        -> std::pair<Iterator, std::optional<std::string>>
    {
        auto word_p = word_parser();
        auto [pos, word_result] = word_p.parse(begin, end);

        if (!word_result) {
            return {begin, std::nullopt};
        }

        // 1. Exact match
        if (*word_result == target) {
            return {pos, std::move(word_result)};
        }

        // 2. Case-insensitive match
        std::string word_lower = to_lower(*word_result);
        if (word_lower == target_lower) {
            return {pos, std::move(word_result)};
        }

        // 3. Phonetic match
        if (phonetic::soundex(*word_result) == target_soundex) {
            return {pos, std::move(word_result)};
        }

        // 4. Fuzzy match (edit distance)
        if (similarity::levenshtein_distance(*word_result, target) <= max_distance) {
            return {pos, std::move(word_result)};
        }

        return {begin, std::nullopt};
    }
};

/**
 * @brief Create combined fuzzy matcher (tries all strategies)
 */
inline auto combined_fuzzy(std::string target, size_t max_distance = 2) {
    return CombinedFuzzyMatcher(std::move(target), max_distance);
}

} // namespace fuzzy
} // namespace alga

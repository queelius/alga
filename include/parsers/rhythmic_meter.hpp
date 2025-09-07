#pragma once

#include "lc_alpha.hpp"
#include "porter2stemmer.hpp"
#include "algebraic_operators.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <optional>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>

namespace algebraic_parsers {

/**
 * @brief Stress level enumeration for syllabic analysis
 * 
 * Models the fundamental acoustic properties that create rhythmic patterns.
 */
enum class stress_level {
    unstressed = 0,  // ˘ - weak syllable
    primary = 1,     // ˊ - strong syllable  
    secondary = 2    // ˈ - medium stress (compound words)
};

/**
 * @brief Syllable with stress information and algebraic properties
 * 
 * Forms the atomic unit of rhythmic analysis. Syllables compose
 * through concatenation to form larger rhythmic patterns.
 */
struct stressed_syllable {
    lc_alpha phonetic_content;  // The actual sound content
    stress_level stress;        // Prosodic stress level
    double duration;            // Relative temporal duration
    
    // Constructors
    stressed_syllable() = default;
    stressed_syllable(lc_alpha content, stress_level s, double d = 1.0) 
        : phonetic_content(std::move(content)), stress(s), duration(d) {}
    
    // Value semantics
    stressed_syllable(stressed_syllable const&) = default;
    stressed_syllable(stressed_syllable&&) = default;
    stressed_syllable& operator=(stressed_syllable const&) = default;
    stressed_syllable& operator=(stressed_syllable&&) = default;
    
    // Comparison based on stress pattern only
    bool operator==(stressed_syllable const& other) const {
        return stress == other.stress;
    }
    
    bool empty() const { 
        return phonetic_content.empty(); 
    }
    
    // Access interface
    std::string str() const { return phonetic_content.str(); }
    explicit operator std::string() const { return str(); }
};

/**
 * @brief Rhythmic pattern as sequence of stressed syllables
 * 
 * Represents a complete rhythmic unit that can be analyzed, compared,
 * and composed using algebraic operations. Forms a monoid under concatenation.
 */
class rhythmic_pattern {
private:
    std::vector<stressed_syllable> syllables;
    
public:
    // Constructors
    rhythmic_pattern() = default;
    rhythmic_pattern(std::vector<stressed_syllable> syls) : syllables(std::move(syls)) {}
    rhythmic_pattern(std::initializer_list<stressed_syllable> syls) : syllables(syls) {}
    
    // Value semantics
    rhythmic_pattern(rhythmic_pattern const&) = default;
    rhythmic_pattern(rhythmic_pattern&&) = default;
    rhythmic_pattern& operator=(rhythmic_pattern const&) = default;
    rhythmic_pattern& operator=(rhythmic_pattern&&) = default;
    
    // Access interface
    std::vector<stressed_syllable> const& get_syllables() const { return syllables; }
    bool empty() const { return syllables.empty(); }
    size_t size() const { return syllables.size(); }
    
    // Iterator support
    auto begin() const { return syllables.begin(); }
    auto end() const { return syllables.end(); }
    
    // Stress pattern extraction (for pattern matching)
    std::vector<stress_level> stress_pattern() const {
        std::vector<stress_level> pattern;
        pattern.reserve(syllables.size());
        std::transform(syllables.begin(), syllables.end(), 
                      std::back_inserter(pattern),
                      [](auto const& syl) { return syl.stress; });
        return pattern;
    }
    
    // Rhythmic metrics
    double total_duration() const {
        return std::accumulate(syllables.begin(), syllables.end(), 0.0,
                              [](double sum, auto const& syl) { 
                                  return sum + syl.duration; 
                              });
    }
    
    // Stress density (ratio of stressed to total syllables)
    double stress_density() const {
        if (syllables.empty()) return 0.0;
        auto stressed_count = std::count_if(syllables.begin(), syllables.end(),
            [](auto const& syl) { return syl.stress != stress_level::unstressed; });
        return static_cast<double>(stressed_count) / syllables.size();
    }
    
    // Rhythmic regularity measure (lower = more regular)
    double regularity_metric() const {
        if (syllables.size() < 2) return 0.0;
        
        std::vector<double> intervals;
        for (size_t i = 1; i < syllables.size(); ++i) {
            intervals.push_back(syllables[i].duration);
        }
        
        // Calculate variance in interval durations
        double mean = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();
        double variance = 0.0;
        for (double interval : intervals) {
            variance += (interval - mean) * (interval - mean);
        }
        return variance / intervals.size();
    }
};

/**
 * @brief Factory function for creating rhythmic patterns from text
 */
std::optional<rhythmic_pattern> make_rhythmic_pattern(std::string_view text);

/**
 * @brief Monoid composition for rhythmic patterns
 * 
 * Concatenates two rhythmic patterns, preserving the temporal sequence.
 */
rhythmic_pattern operator*(rhythmic_pattern const& lhs, rhythmic_pattern const& rhs) {
    auto combined = lhs.get_syllables();
    auto rhs_syls = rhs.get_syllables();
    combined.insert(combined.end(), rhs_syls.begin(), rhs_syls.end());
    return rhythmic_pattern(std::move(combined));
}

/**
 * @brief Optional monadic composition
 */
std::optional<rhythmic_pattern> operator*(std::optional<rhythmic_pattern> const& lhs, 
                                         std::optional<rhythmic_pattern> const& rhs) {
    if (!lhs || !rhs) return std::nullopt;
    return *lhs * *rhs;
}

/**
 * @brief Choice operator - prefer the pattern with higher stress density
 */
rhythmic_pattern operator|(rhythmic_pattern const& lhs, rhythmic_pattern const& rhs) {
    return lhs.stress_density() >= rhs.stress_density() ? lhs : rhs;
}

/**
 * @brief Choice operator for optionals
 */
std::optional<rhythmic_pattern> operator|(std::optional<rhythmic_pattern> const& lhs,
                                         std::optional<rhythmic_pattern> const& rhs) {
    return lhs ? lhs : rhs;
}

/**
 * @brief Repetition operator - creates repeated rhythmic patterns
 */
rhythmic_pattern operator^(rhythmic_pattern const& base, size_t count) {
    if (count == 0) return rhythmic_pattern{};
    if (count == 1) return base;
    
    auto result = base;
    for (size_t i = 1; i < count; ++i) {
        result = result * base;
    }
    return result;
}

/**
 * @brief Function application operator for rhythmic analysis
 */
template<typename F>
    requires std::invocable<F, rhythmic_pattern>
auto operator%(rhythmic_pattern const& pattern, F const& function) 
    -> decltype(function(pattern)) {
    return function(pattern);
}

template<typename F>
    requires std::invocable<F, rhythmic_pattern>
auto operator%(std::optional<rhythmic_pattern> const& maybe_pattern, F const& function)
    -> std::optional<std::decay_t<decltype(function(*maybe_pattern))>> {
    if (!maybe_pattern) return std::nullopt;
    return function(*maybe_pattern);
}

/**
 * @brief Comparison operators
 */
bool operator==(rhythmic_pattern const& lhs, rhythmic_pattern const& rhs) {
    return lhs.stress_pattern() == rhs.stress_pattern();
}

bool operator!=(rhythmic_pattern const& lhs, rhythmic_pattern const& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Stream output for rhythmic patterns
 */
std::ostream& operator<<(std::ostream& os, rhythmic_pattern const& pattern) {
    os << "Rhythm[";
    bool first = true;
    for (auto const& syllable : pattern) {
        if (!first) os << "-";
        switch (syllable.stress) {
            case stress_level::unstressed: os << "˘"; break;
            case stress_level::primary: os << "ˊ"; break;
            case stress_level::secondary: os << "ˈ"; break;
        }
        first = false;
    }
    os << "]";
    return os;
}

/**
 * @brief Rhythmic meter analyzer with algebraic composition
 * 
 * Analyzes text for rhythmic patterns and provides compositional
 * operations for building complex rhythmic analyses.
 */
class rhythmic_meter_analyzer {
public:
    using input_type = std::string_view;
    using output_type = rhythmic_pattern;
    
    /**
     * @brief Analyze text for rhythmic patterns
     */
    std::optional<rhythmic_pattern> operator()(std::string_view text) const {
        return make_rhythmic_pattern(text);
    }
    
    /**
     * @brief Iterator-based parsing for composition
     */
    template<typename Iterator>
    std::pair<Iterator, std::optional<rhythmic_pattern>> parse(Iterator begin, Iterator end) const {
        std::string word;
        Iterator current = begin;
        
        // Extract word characters
        while (current != end && (std::isalpha(*current) || std::isspace(*current))) {
            if (std::isalpha(*current)) {
                word += std::tolower(*current);
            } else if (!word.empty()) {
                // End of word, analyze it
                break;
            }
            ++current;
        }
        
        if (word.empty()) {
            return {current, std::nullopt};
        }
        
        auto pattern = make_rhythmic_pattern(word);
        return {current, pattern};
    }
};

/**
 * @brief Predefined classical meter patterns for pattern matching
 */
namespace classical_meters {
    // Basic feet (building blocks of meter)
    inline rhythmic_pattern iamb() {
        return rhythmic_pattern{{
            {make_lc_alpha("da").value_or(lc_alpha{}), stress_level::unstressed},
            {make_lc_alpha("dum").value_or(lc_alpha{}), stress_level::primary}
        }};
    }
    
    inline rhythmic_pattern trochee() {
        return rhythmic_pattern{{
            {make_lc_alpha("dum").value_or(lc_alpha{}), stress_level::primary},
            {make_lc_alpha("da").value_or(lc_alpha{}), stress_level::unstressed}
        }};
    }
    
    inline rhythmic_pattern anapest() {
        return rhythmic_pattern{{
            {make_lc_alpha("da").value_or(lc_alpha{}), stress_level::unstressed},
            {make_lc_alpha("da").value_or(lc_alpha{}), stress_level::unstressed},
            {make_lc_alpha("dum").value_or(lc_alpha{}), stress_level::primary}
        }};
    }
    
    inline rhythmic_pattern dactyl() {
        return rhythmic_pattern{{
            {make_lc_alpha("dum").value_or(lc_alpha{}), stress_level::primary},
            {make_lc_alpha("da").value_or(lc_alpha{}), stress_level::unstressed},
            {make_lc_alpha("da").value_or(lc_alpha{}), stress_level::unstressed}
        }};
    }
    
    // Common meter patterns using algebraic composition
    inline rhythmic_pattern iambic_pentameter() {
        return iamb() ^ 5;  // Five iambs
    }
    
    inline rhythmic_pattern trochaic_tetrameter() {
        return trochee() ^ 4;  // Four trochees
    }
    
    inline rhythmic_pattern anapestic_trimeter() {
        return anapest() ^ 3;  // Three anapests
    }
    
    inline rhythmic_pattern dactylic_hexameter() {
        return dactyl() ^ 6;  // Six dactyls (classical epic meter)
    }
    
    /**
     * @brief Meter classification function using algebraic pattern matching
     */
    inline std::string classify_meter(rhythmic_pattern const& pattern) {
        // Use choice operator to find best match
        auto pentameter_score = [&]() {
            auto canonical = iambic_pentameter();
            return pattern.size() == canonical.size() && 
                   pattern.stress_density() > 0.4 && pattern.stress_density() < 0.6;
        };
        
        auto tetrameter_score = [&]() {
            auto canonical = trochaic_tetrameter();
            return pattern.size() == canonical.size() &&
                   pattern.stress_density() > 0.4 && pattern.stress_density() < 0.6;
        };
        
        if (pentameter_score()) return "Iambic Pentameter";
        if (tetrameter_score()) return "Trochaic Tetrameter";
        if (pattern.stress_density() > 0.7) return "Heavy Stress";
        if (pattern.stress_density() < 0.3) return "Light Stress";
        return "Free Verse";
    }
}

/**
 * @brief Helper function to detect vowels
 */
inline bool is_vowel(char c) {
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y';
}

/**
 * @brief Implementation of rhythmic pattern factory function
 * 
 * Simplified syllable detection and stress assignment for demonstration.
 * In a production system, this would use sophisticated phonetic analysis.
 */
std::optional<rhythmic_pattern> make_rhythmic_pattern(std::string_view text) {
    if (text.empty()) return rhythmic_pattern{};
    
    // Convert to lowercase for processing
    std::string lower_text;
    std::transform(text.begin(), text.end(), std::back_inserter(lower_text),
                   [](char c) { return std::tolower(c); });
    
    std::vector<stressed_syllable> syllables;
    std::string current_syllable;
    
    // Simple syllable detection based on vowel patterns
    // This is a simplified heuristic - real implementation would use phonetic libraries
    for (size_t i = 0; i < lower_text.length(); ++i) {
        char c = lower_text[i];
        
        if (!std::isalpha(c)) {
            if (!current_syllable.empty()) {
                // End current syllable
                auto lc_content = make_lc_alpha(current_syllable);
                if (lc_content) {
                    // Simple stress assignment heuristic
                    stress_level stress = (current_syllable.length() > 3 || 
                                         (syllables.size() % 2 == 1)) ? 
                                        stress_level::primary : stress_level::unstressed;
                    
                    syllables.emplace_back(*lc_content, stress, 1.0);
                }
                current_syllable.clear();
            }
            continue;
        }
        
        current_syllable += c;
        
        // Simple vowel-based syllable boundary detection
        if (i > 0 && is_vowel(c) && !is_vowel(lower_text[i-1])) {
            // Potential syllable boundary - simplified logic
            if (current_syllable.length() > 2) {
                auto lc_content = make_lc_alpha(current_syllable.substr(0, current_syllable.length()-1));
                if (lc_content) {
                    stress_level stress = (syllables.size() % 2 == 0) ? 
                                        stress_level::primary : stress_level::unstressed;
                    syllables.emplace_back(*lc_content, stress, 1.0);
                }
                current_syllable = c; // Start new syllable with current vowel
            }
        }
    }
    
    // Add final syllable if present
    if (!current_syllable.empty()) {
        auto lc_content = make_lc_alpha(current_syllable);
        if (lc_content) {
            stress_level stress = (current_syllable.length() > 3) ? 
                                stress_level::primary : stress_level::unstressed;
            syllables.emplace_back(*lc_content, stress, 1.0);
        }
    }
    
    if (syllables.empty()) return std::nullopt;
    
    return rhythmic_pattern(std::move(syllables));
}

} // namespace algebraic_parsers
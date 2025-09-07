#pragma once

#include "lc_alpha.hpp"
#include "porter2stemmer.hpp"
#include "semantic_topology.hpp"
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
#include <complex>

namespace alga {

/**
 * @brief Narrative element types for structural analysis
 * 
 * Models the fundamental components that compose to create narrative structure.
 */
enum class narrative_element_type {
    exposition,      // Setting up the world and characters
    inciting_incident, // Event that sets the story in motion
    rising_action,   // Building tension and complexity
    climax,          // Peak of narrative tension
    falling_action,  // Consequences of the climax
    resolution,      // Final state and closure
    character_moment, // Character development or revelation
    dialogue,        // Character interaction
    description,     // Scene or atmosphere setting
    reflection,      // Internal thoughts or commentary
    transition,      // Bridge between scenes or sections
    foreshadowing,   // Hints at future events
    flashback,       // Reference to past events
    comic_relief     // Tension release through humor
};

/**
 * @brief Tension level for dramatic structure analysis
 * 
 * Represents the emotional/dramatic intensity at any point in the narrative.
 * Uses complex numbers to capture both magnitude and emotional tone.
 */
struct tension_level {
    std::complex<double> intensity;  // Magnitude = tension, phase = emotional tone
    double sustainability;           // How long this tension level can be maintained
    
    tension_level() : intensity(0.0, 0.0), sustainability(1.0) {}
    tension_level(double mag, double phase = 0.0, double sustain = 1.0) 
        : intensity(mag * std::cos(phase), mag * std::sin(phase)), sustainability(sustain) {}
    tension_level(std::complex<double> inten, double sustain = 1.0)
        : intensity(inten), sustainability(sustain) {}
    
    // Magnitude (tension strength)
    double magnitude() const { return std::abs(intensity); }
    
    // Phase (emotional coloring: 0 = neutral, π/2 = uplifting, π = tragic, etc.)
    double phase() const { return std::arg(intensity); }
    
    // Algebraic operations on tension
    tension_level operator+(tension_level const& other) const {
        return tension_level(
            intensity + other.intensity,
            std::min(sustainability, other.sustainability)
        );
    }
    
    tension_level operator*(double factor) const {
        return tension_level(intensity * factor, sustainability * factor);
    }
    
    // Decay over time
    tension_level decay(double time_factor) const {
        double decay_factor = std::exp(-time_factor / sustainability);
        return tension_level(intensity * decay_factor, sustainability);
    }
};

/**
 * @brief Character state representing psychological and narrative position
 * 
 * Tracks character development through multidimensional character space.
 */
struct character_state {
    std::map<std::string, double> traits;     // Character attributes (-1 to 1)
    std::map<std::string, double> relationships; // Relationship strengths
    semantic_vector motivation;               // Current driving force
    double agency;                           // Character's ability to affect plot
    double arc_progress;                     // Progress through character arc (0 to 1)
    
    character_state() : agency(0.5), arc_progress(0.0) {}
    
    // Character development through story events
    character_state operator+(character_state const& development) const {
        character_state result = *this;
        
        // Merge traits (weighted average)
        for (auto const& [trait, value] : development.traits) {
            double current = result.traits.count(trait) ? result.traits[trait] : 0.0;
            result.traits[trait] = (current + value) * 0.5;
        }
        
        // Update relationships
        for (auto const& [rel, strength] : development.relationships) {
            double current = result.relationships.count(rel) ? result.relationships[rel] : 0.0;
            result.relationships[rel] = std::clamp(current + strength, -1.0, 1.0);
        }
        
        // Combine motivations
        result.motivation = motivation * development.motivation;
        
        // Update agency and arc progress
        result.agency = std::clamp(agency + development.agency, 0.0, 1.0);
        result.arc_progress = std::clamp(arc_progress + development.arc_progress, 0.0, 1.0);
        
        return result;
    }
    
    // Character similarity for ensemble analysis
    double similarity(character_state const& other) const {
        double trait_similarity = 0.0;
        size_t common_traits = 0;
        
        for (auto const& [trait, value] : traits) {
            if (other.traits.count(trait)) {
                trait_similarity += 1.0 - std::abs(value - other.traits.at(trait)) * 0.5;
                ++common_traits;
            }
        }
        
        if (common_traits == 0) return 0.0;
        return trait_similarity / common_traits;
    }
};

/**
 * @brief Thematic content representing the deeper meaning layer
 * 
 * Captures the abstract concepts and themes that give narrative its significance.
 */
struct thematic_content {
    std::vector<semantic_vector> themes;     // Core thematic concepts
    std::map<std::string, double> symbols;  // Symbolic elements and their weights
    double universality;                     // How broadly applicable the themes are
    double coherence;                        // How well themes work together
    
    thematic_content() : universality(0.5), coherence(1.0) {}
    thematic_content(std::vector<semantic_vector> t) : themes(std::move(t)), universality(0.5), coherence(1.0) {}
    
    // Thematic composition
    thematic_content operator*(thematic_content const& other) const {
        thematic_content result;
        result.themes = themes;
        result.themes.insert(result.themes.end(), other.themes.begin(), other.themes.end());
        
        // Merge symbols
        result.symbols = symbols;
        for (auto const& [symbol, weight] : other.symbols) {
            result.symbols[symbol] = result.symbols.count(symbol) ? 
                (result.symbols[symbol] + weight) * 0.5 : weight;
        }
        
        // Calculate combined properties
        result.universality = (universality + other.universality) * 0.5;
        result.coherence = coherence * other.coherence; // Coherence multiplies
        
        return result;
    }
    
    // Thematic density (themes per unit of content)
    double density() const {
        return static_cast<double>(themes.size()) / (1.0 + symbols.size());
    }
    
    // Thematic resonance (how well themes reinforce each other)
    double resonance() const {
        if (themes.size() < 2) return 1.0;
        
        double total_similarity = 0.0;
        size_t pairs = 0;
        
        for (size_t i = 0; i < themes.size(); ++i) {
            for (size_t j = i + 1; j < themes.size(); ++j) {
                total_similarity += themes[i].similarity(themes[j]);
                ++pairs;
            }
        }
        
        return pairs > 0 ? total_similarity / pairs : 0.0;
    }
};

/**
 * @brief Narrative element as the atomic unit of story structure
 * 
 * Represents a single meaningful unit of narrative that can be analyzed
 * and composed with others to form larger narrative structures.
 */
class narrative_element {
private:
    narrative_element_type type;
    tension_level tension;
    character_state character_impact;
    thematic_content themes;
    porter2_stem content;      // The actual text content
    double duration;           // Relative temporal length
    double causal_weight;      // How much this element affects future events
    
public:
    // Constructors
    narrative_element() : type(narrative_element_type::exposition), duration(1.0), causal_weight(0.5) {}
    
    narrative_element(narrative_element_type t, tension_level ten, porter2_stem cont)
        : type(t), tension(ten), content(std::move(cont)), duration(1.0), causal_weight(0.5) {}
    
    narrative_element(narrative_element_type t, tension_level ten, character_state chars, 
                     thematic_content them, porter2_stem cont)
        : type(t), tension(ten), character_impact(chars), themes(them), 
          content(std::move(cont)), duration(1.0), causal_weight(0.5) {}
    
    // Value semantics
    narrative_element(narrative_element const&) = default;
    narrative_element(narrative_element&&) = default;
    narrative_element& operator=(narrative_element const&) = default;
    narrative_element& operator=(narrative_element&&) = default;
    
    // Access interface
    narrative_element_type get_type() const { return type; }
    tension_level const& get_tension() const { return tension; }
    character_state const& get_character_impact() const { return character_impact; }
    thematic_content const& get_themes() const { return themes; }
    porter2_stem const& get_content() const { return content; }
    double get_duration() const { return duration; }
    double get_causal_weight() const { return causal_weight; }
    
    // Setters for narrative analysis
    void set_tension(tension_level t) { tension = t; }
    void set_character_impact(character_state cs) { character_impact = cs; }
    void set_themes(thematic_content tc) { themes = tc; }
    void set_duration(double d) { duration = std::max(0.1, d); }
    void set_causal_weight(double cw) { causal_weight = std::clamp(cw, 0.0, 1.0); }
    
    bool empty() const { return content.empty(); }
    
    // Narrative compatibility (how well elements flow together)
    double compatibility(narrative_element const& next) const {
        // Type compatibility matrix (simplified)
        static const std::map<std::pair<narrative_element_type, narrative_element_type>, double> 
        compatibility_matrix = {
            {{narrative_element_type::exposition, narrative_element_type::inciting_incident}, 0.9},
            {{narrative_element_type::inciting_incident, narrative_element_type::rising_action}, 0.95},
            {{narrative_element_type::rising_action, narrative_element_type::climax}, 0.9},
            {{narrative_element_type::climax, narrative_element_type::falling_action}, 0.85},
            {{narrative_element_type::falling_action, narrative_element_type::resolution}, 0.9},
            // Add more compatibility rules...
        };
        
        auto key = std::make_pair(type, next.type);
        double type_compat = compatibility_matrix.count(key) ? compatibility_matrix.at(key) : 0.5;
        
        // Tension compatibility (smooth transitions preferred)
        double tension_diff = std::abs(tension.magnitude() - next.tension.magnitude());
        double tension_compat = 1.0 / (1.0 + tension_diff);
        
        // Thematic compatibility
        double theme_compat = themes.resonance() * next.themes.resonance();
        
        return (type_compat + tension_compat + theme_compat) / 3.0;
    }
};

/**
 * @brief Narrative structure as composition of elements
 * 
 * Represents a complete narrative arc formed by algebraic composition
 * of narrative elements. Supports analysis of dramatic structure.
 */
class narrative_structure {
private:
    std::vector<narrative_element> elements;
    mutable bool analysis_cached = false;
    mutable double total_tension_cache = 0.0;
    mutable std::vector<double> tension_curve_cache;
    
    void invalidate_cache() const { analysis_cached = false; }
    
public:
    // Constructors
    narrative_structure() = default;
    narrative_structure(std::vector<narrative_element> elems) : elements(std::move(elems)) {}
    narrative_structure(std::initializer_list<narrative_element> elems) : elements(elems) {}
    
    // Value semantics
    narrative_structure(narrative_structure const&) = default;
    narrative_structure(narrative_structure&&) = default;
    narrative_structure& operator=(narrative_structure const&) = default;
    narrative_structure& operator=(narrative_structure&&) = default;
    
    // Access interface
    std::vector<narrative_element> const& get_elements() const { return elements; }
    bool empty() const { return elements.empty(); }
    size_t size() const { return elements.size(); }
    
    // Iterator support
    auto begin() const { return elements.begin(); }
    auto end() const { return elements.end(); }
    
    // Add element to structure
    void add_element(narrative_element elem) {
        elements.push_back(std::move(elem));
        invalidate_cache();
    }
    
    // Dramatic structure analysis
    std::vector<double> tension_curve() const {
        if (!analysis_cached) {
            calculate_tension_analysis();
        }
        return tension_curve_cache;
    }
    
    double peak_tension() const {
        auto curve = tension_curve();
        return curve.empty() ? 0.0 : *std::max_element(curve.begin(), curve.end());
    }
    
    size_t climax_position() const {
        auto curve = tension_curve();
        if (curve.empty()) return 0;
        return std::distance(curve.begin(), std::max_element(curve.begin(), curve.end()));
    }
    
    // Narrative coherence (how well the story holds together)
    double coherence() const {
        if (elements.size() < 2) return 1.0;
        
        double total_compatibility = 0.0;
        for (size_t i = 0; i < elements.size() - 1; ++i) {
            total_compatibility += elements[i].compatibility(elements[i + 1]);
        }
        
        return total_compatibility / (elements.size() - 1);
    }
    
    // Pacing analysis (variation in element durations)
    double pacing_variance() const {
        if (elements.empty()) return 0.0;
        
        std::vector<double> durations;
        durations.reserve(elements.size());
        std::transform(elements.begin(), elements.end(), std::back_inserter(durations),
                      [](auto const& elem) { return elem.get_duration(); });
        
        double mean = std::accumulate(durations.begin(), durations.end(), 0.0) / durations.size();
        double variance = 0.0;
        for (double duration : durations) {
            variance += (duration - mean) * (duration - mean);
        }
        
        return variance / durations.size();
    }
    
    // Character arc analysis
    std::map<std::string, double> character_development() const {
        std::map<std::string, double> total_development;
        
        for (auto const& elem : elements) {
            auto const& char_state = elem.get_character_impact();
            for (auto const& [character, development] : char_state.relationships) {
                total_development[character] = total_development.count(character) ?
                    total_development[character] + development : development;
            }
        }
        
        return total_development;
    }
    
    // Thematic density over narrative
    double thematic_density() const {
        if (elements.empty()) return 0.0;
        
        size_t total_themes = 0;
        for (auto const& elem : elements) {
            total_themes += elem.get_themes().themes.size();
        }
        
        return static_cast<double>(total_themes) / elements.size();
    }
    
private:
    void calculate_tension_analysis() const {
        tension_curve_cache.clear();
        if (elements.empty()) {
            analysis_cached = true;
            return;
        }
        
        tension_curve_cache.reserve(elements.size());
        double cumulative_tension = 0.0;
        
        for (auto const& elem : elements) {
            cumulative_tension += elem.get_tension().magnitude();
            tension_curve_cache.push_back(cumulative_tension);
        }
        
        total_tension_cache = cumulative_tension;
        analysis_cached = true;
    }
};

/**
 * @brief Factory function for creating narrative elements from text
 */
std::optional<narrative_element> make_narrative_element(std::string_view text, 
                                                       narrative_element_type type = narrative_element_type::exposition);

/**
 * @brief Monoid composition for narrative elements (sequential narrative)
 * 
 * Combines two narrative elements into a sequential structure,
 * accounting for causal relationships and tension flow.
 */
narrative_element operator*(narrative_element const& lhs, narrative_element const& rhs) {
    // Create composite element that represents the sequence
    auto combined_content = lhs.get_content() * rhs.get_content();
    
    // Tension evolves: second element's tension modified by first's causal weight
    tension_level evolved_tension = rhs.get_tension();
    evolved_tension = evolved_tension * (1.0 + lhs.get_causal_weight());
    
    // Character development accumulates
    character_state combined_chars = lhs.get_character_impact() + rhs.get_character_impact();
    
    // Themes combine
    thematic_content combined_themes = lhs.get_themes() * rhs.get_themes();
    
    // Duration adds up
    double combined_duration = lhs.get_duration() + rhs.get_duration();
    
    // Create result with combined properties
    narrative_element result(rhs.get_type(), evolved_tension, combined_chars, 
                           combined_themes, combined_content);
    result.set_duration(combined_duration);
    result.set_causal_weight((lhs.get_causal_weight() + rhs.get_causal_weight()) * 0.5);
    
    return result;
}

/**
 * @brief Structure composition
 */
narrative_structure operator*(narrative_structure const& lhs, narrative_structure const& rhs) {
    auto combined = lhs.get_elements();
    auto rhs_elements = rhs.get_elements();
    combined.insert(combined.end(), rhs_elements.begin(), rhs_elements.end());
    return narrative_structure(std::move(combined));
}

/**
 * @brief Choice operators for narrative alternatives
 */
narrative_element operator|(narrative_element const& lhs, narrative_element const& rhs) {
    // Choose element with higher dramatic tension
    return lhs.get_tension().magnitude() >= rhs.get_tension().magnitude() ? lhs : rhs;
}

narrative_structure operator|(narrative_structure const& lhs, narrative_structure const& rhs) {
    // Choose structure with better coherence
    return lhs.coherence() >= rhs.coherence() ? lhs : rhs;
}

/**
 * @brief Repetition operators for narrative patterns
 */
narrative_element operator^(narrative_element const& base, size_t count) {
    if (count == 0) return narrative_element{};
    if (count == 1) return base;
    
    // Create intensified version through repetition
    auto result = base;
    result.set_causal_weight(std::clamp(base.get_causal_weight() * count, 0.0, 1.0));
    result.set_duration(base.get_duration() * count);
    
    // Tension amplifies with repetition
    tension_level amplified = base.get_tension();
    amplified = amplified * std::sqrt(count); // Square root to prevent explosion
    result.set_tension(amplified);
    
    return result;
}

/**
 * @brief Narrative algebra analyzer
 * 
 * Analyzes text for narrative structure and provides compositional
 * operations for building complex narrative analyses.
 */
class narrative_algebra_analyzer {
public:
    using input_type = std::string_view;
    using output_type = narrative_element;
    
    /**
     * @brief Analyze text for narrative element
     */
    std::optional<narrative_element> operator()(std::string_view text) const {
        return make_narrative_element(text);
    }
    
    /**
     * @brief Analyze complete narrative structure
     */
    std::optional<narrative_structure> analyze_structure(std::vector<std::string_view> const& segments) const {
        std::vector<narrative_element> elements;
        
        for (auto segment : segments) {
            auto element = make_narrative_element(segment);
            if (element) {
                elements.push_back(*element);
            }
        }
        
        return elements.empty() ? std::nullopt : 
               std::make_optional(narrative_structure(std::move(elements)));
    }
    
    /**
     * @brief Iterator-based parsing for composition
     */
    template<typename Iterator>
    std::pair<Iterator, std::optional<narrative_element>> parse(Iterator begin, Iterator end) const {
        std::string sentence;
        Iterator current = begin;
        
        // Extract sentence (simplified - look for punctuation)
        while (current != end && *current != '.' && *current != '!' && *current != '?') {
            sentence += *current;
            ++current;
        }
        
        if (current != end) ++current; // Skip punctuation
        
        if (sentence.empty()) {
            return {current, std::nullopt};
        }
        
        auto element = make_narrative_element(sentence);
        return {current, element};
    }
};

/**
 * @brief Classical narrative patterns using algebraic composition
 */
namespace narrative_patterns {
    // Basic dramatic structure templates
    inline narrative_structure three_act_structure() {
        auto setup = narrative_element(narrative_element_type::exposition, 
                                     tension_level(0.2), 
                                     make_porter2_stem("setup").value_or(porter2_stem{}));
        
        auto conflict = narrative_element(narrative_element_type::rising_action,
                                        tension_level(0.7),
                                        make_porter2_stem("conflict").value_or(porter2_stem{}));
        
        auto resolution = narrative_element(narrative_element_type::resolution,
                                          tension_level(0.1),
                                          make_porter2_stem("resolution").value_or(porter2_stem{}));
        
        return narrative_structure{setup, conflict, resolution};
    }
    
    inline narrative_structure heros_journey() {
        std::vector<narrative_element> journey;
        
        // Simplified hero's journey stages
        std::vector<std::pair<narrative_element_type, double>> stages = {
            {narrative_element_type::exposition, 0.1},      // Ordinary world
            {narrative_element_type::inciting_incident, 0.3}, // Call to adventure
            {narrative_element_type::rising_action, 0.5},     // Crossing threshold
            {narrative_element_type::rising_action, 0.7},     // Trials
            {narrative_element_type::climax, 0.9},            // Ordeal
            {narrative_element_type::falling_action, 0.6},    // Reward
            {narrative_element_type::falling_action, 0.4},    // Road back
            {narrative_element_type::resolution, 0.2}         // Return transformed
        };
        
        for (auto const& [type, tension_mag] : stages) {
            auto content = make_porter2_stem("journey_stage").value_or(porter2_stem{});
            journey.emplace_back(type, tension_level(tension_mag), content);
        }
        
        return narrative_structure(std::move(journey));
    }
    
    inline narrative_structure tragedy_pattern() {
        auto hubris = narrative_element(narrative_element_type::character_moment,
                                      tension_level(0.8, M_PI), // Phase = π for tragic tone
                                      make_porter2_stem("hubris").value_or(porter2_stem{}));
        
        auto fall = narrative_element(narrative_element_type::climax,
                                    tension_level(1.0, M_PI),
                                    make_porter2_stem("downfall").value_or(porter2_stem{}));
        
        auto catharsis = narrative_element(narrative_element_type::resolution,
                                         tension_level(0.3, M_PI/2), // Phase = π/2 for bittersweet
                                         make_porter2_stem("catharsis").value_or(porter2_stem{}));
        
        return narrative_structure{hubris, fall, catharsis};
    }
    
    /**
     * @brief Classify narrative structure using pattern matching
     */
    inline std::string classify_narrative(narrative_structure const& structure) {
        auto curve = structure.tension_curve();
        if (curve.empty()) return "Fragment";
        
        double peak_pos = static_cast<double>(structure.climax_position()) / curve.size();
        double peak_tension = structure.peak_tension();
        double coherence = structure.coherence();
        
        if (coherence > 0.8 && peak_pos > 0.7 && peak_tension > 0.8) {
            return "Classical Three-Act";
        } else if (coherence > 0.7 && curve.size() > 5 && peak_pos > 0.6) {
            return "Hero's Journey";
        } else if (peak_tension > 0.9 && peak_pos < 0.8) {
            return "Tragedy";
        } else if (structure.pacing_variance() < 0.1) {
            return "Linear Narrative";
        } else if (coherence < 0.5) {
            return "Experimental";
        } else {
            return "Contemporary";
        }
    }
}

/**
 * @brief Implementation of narrative element factory function
 */
std::optional<narrative_element> make_narrative_element(std::string_view text, 
                                                       narrative_element_type type) {
    if (text.empty()) return std::nullopt;
    
    auto content = make_porter2_stem(text);
    if (!content) return std::nullopt;
    
    // Heuristic analysis of text to determine narrative properties
    std::string lower_text;
    std::transform(text.begin(), text.end(), std::back_inserter(lower_text),
                   [](char c) { return std::tolower(c); });
    
    // Simple tension analysis based on content
    double tension_magnitude = 0.3; // Base tension
    
    // Action words increase tension
    if (lower_text.find("fight") != std::string::npos || 
        lower_text.find("run") != std::string::npos ||
        lower_text.find("attack") != std::string::npos) {
        tension_magnitude += 0.4;
    }
    
    // Emotional words affect tension
    if (lower_text.find("fear") != std::string::npos ||
        lower_text.find("danger") != std::string::npos ||
        lower_text.find("crisis") != std::string::npos) {
        tension_magnitude += 0.3;
    }
    
    // Dialogue reduces base tension slightly
    if (lower_text.find("said") != std::string::npos ||
        lower_text.find("\"") != std::string::npos) {
        tension_magnitude *= 0.8;
        type = narrative_element_type::dialogue;
    }
    
    tension_level tension(std::clamp(tension_magnitude, 0.0, 1.0));
    
    // Create basic character state and themes (simplified)
    character_state chars;
    chars.agency = tension_magnitude * 0.5; // More tension often means more agency
    
    thematic_content themes;
    themes.universality = 0.5; // Default universality
    
    return narrative_element(type, tension, chars, themes, *content);
}

} // namespace alga
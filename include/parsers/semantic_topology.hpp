#pragma once

#include "lc_alpha.hpp"
#include "porter2stemmer.hpp"
#include "algebraic_operators.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iomanip>

namespace alga {

/**
 * @brief Semantic dimension enumeration for conceptual analysis
 * 
 * Models the fundamental axes along which concepts can be differentiated.
 * These form the basis vectors of semantic space.
 */
enum class semantic_dimension {
    concreteness,      // Abstract ← → Concrete
    positivity,        // Negative ← → Positive  
    complexity,        // Simple ← → Complex
    temporality,       // Atemporal ← → Temporal
    agency,            // Passive ← → Active
    intensity,         // Mild ← → Intense
    formality,         // Informal ← → Formal
    certainty,         // Uncertain ← → Certain
    social_distance,   // Intimate ← → Formal
    emotional_valence  // Negative ← → Positive emotion
};

/**
 * @brief Semantic vector representing concept position in multidimensional space
 * 
 * Each concept exists as a point in semantic space with coordinates along
 * various conceptual dimensions. Forms the foundation for topological operations.
 */
class semantic_vector {
private:
    std::map<semantic_dimension, double> coordinates;
    porter2_stem stem_content;  // The actual semantic content
    mutable double magnitude_cache = -1.0; // Cached magnitude for efficiency
    
    void invalidate_cache() { magnitude_cache = -1.0; }
    
public:
    // Constructors
    semantic_vector() = default;
    semantic_vector(porter2_stem content) : stem_content(std::move(content)) {}
    semantic_vector(porter2_stem content, std::map<semantic_dimension, double> coords) 
        : coordinates(std::move(coords)), stem_content(std::move(content)) {}
    
    // Value semantics
    semantic_vector(semantic_vector const&) = default;
    semantic_vector(semantic_vector&&) = default;
    semantic_vector& operator=(semantic_vector const&) = default;
    semantic_vector& operator=(semantic_vector&&) = default;
    
    // Access interface
    porter2_stem const& content() const { return stem_content; }
    std::map<semantic_dimension, double> const& get_coordinates() const { return coordinates; }
    
    // Coordinate access and modification
    double get_coordinate(semantic_dimension dim) const {
        auto it = coordinates.find(dim);
        return it != coordinates.end() ? it->second : 0.0;
    }
    
    void set_coordinate(semantic_dimension dim, double value) {
        coordinates[dim] = std::clamp(value, -1.0, 1.0); // Normalize to [-1, 1]
        invalidate_cache();
    }
    
    // Geometric properties
    double magnitude() const {
        if (magnitude_cache < 0.0) {
            magnitude_cache = std::sqrt(std::accumulate(
                coordinates.begin(), coordinates.end(), 0.0,
                [](double sum, auto const& pair) {
                    return sum + pair.second * pair.second;
                }));
        }
        return magnitude_cache;
    }
    
    // Normalize vector to unit length
    semantic_vector normalized() const {
        double mag = magnitude();
        if (mag < 1e-10) return *this;
        
        semantic_vector result = *this;
        for (auto& [dim, coord] : result.coordinates) {
            coord /= mag;
        }
        result.magnitude_cache = 1.0;
        return result;
    }
    
    // Euclidean distance in semantic space
    double distance(semantic_vector const& other) const {
        double sum_squares = 0.0;
        std::unordered_set<semantic_dimension> all_dims;
        
        // Collect all dimensions from both vectors
        for (auto const& [dim, _] : coordinates) all_dims.insert(dim);
        for (auto const& [dim, _] : other.coordinates) all_dims.insert(dim);
        
        // Calculate squared differences across all dimensions
        for (semantic_dimension dim : all_dims) {
            double diff = get_coordinate(dim) - other.get_coordinate(dim);
            sum_squares += diff * diff;
        }
        
        return std::sqrt(sum_squares);
    }
    
    // Cosine similarity (angular distance)
    double similarity(semantic_vector const& other) const {
        double dot_product = 0.0;
        std::unordered_set<semantic_dimension> all_dims;
        
        for (auto const& [dim, _] : coordinates) all_dims.insert(dim);
        for (auto const& [dim, _] : other.coordinates) all_dims.insert(dim);
        
        for (semantic_dimension dim : all_dims) {
            dot_product += get_coordinate(dim) * other.get_coordinate(dim);
        }
        
        double mag_product = magnitude() * other.magnitude();
        return (mag_product > 1e-10) ? dot_product / mag_product : 0.0;
    }
    
    bool empty() const { 
        return stem_content.empty(); 
    }
};

/**
 * @brief Semantic cluster representing related concepts in topological space
 * 
 * A collection of semantically related vectors that form a coherent
 * region in semantic space. Supports algebraic operations for composition.
 */
class semantic_cluster {
private:
    std::vector<semantic_vector> concepts;
    mutable semantic_vector centroid_cache;
    mutable bool centroid_valid = false;
    
    void invalidate_centroid() { centroid_valid = false; }
    
public:
    // Constructors
    semantic_cluster() = default;
    semantic_cluster(std::vector<semantic_vector> vecs) : concepts(std::move(vecs)) {}
    semantic_cluster(std::initializer_list<semantic_vector> vecs) : concepts(vecs) {}
    
    // Value semantics
    semantic_cluster(semantic_cluster const&) = default;
    semantic_cluster(semantic_cluster&&) = default;
    semantic_cluster& operator=(semantic_cluster const&) = default;
    semantic_cluster& operator=(semantic_cluster&&) = default;
    
    // Access interface
    std::vector<semantic_vector> const& get_concepts() const { return concepts; }
    bool empty() const { return concepts.empty(); }
    size_t size() const { return concepts.size(); }
    
    // Iterator support
    auto begin() const { return concepts.begin(); }
    auto end() const { return concepts.end(); }
    
    // Add concept to cluster
    void add_concept(semantic_vector semantic_concept) {
        concepts.push_back(std::move(semantic_concept));
        invalidate_centroid();
    }
    
    // Cluster centroid (geometric center of all concepts)
    semantic_vector const& centroid() const {
        if (!centroid_valid) {
            calculate_centroid();
        }
        return centroid_cache;
    }
    
    // Cluster coherence measure (inverse of average intra-cluster distance)
    double coherence() const {
        if (concepts.size() < 2) return 1.0;
        
        double total_distance = 0.0;
        size_t pairs = 0;
        
        for (size_t i = 0; i < concepts.size(); ++i) {
            for (size_t j = i + 1; j < concepts.size(); ++j) {
                total_distance += concepts[i].distance(concepts[j]);
                ++pairs;
            }
        }
        
        double avg_distance = total_distance / pairs;
        return 1.0 / (1.0 + avg_distance); // Normalize to [0, 1]
    }
    
    // Semantic density (concept count per unit volume)
    double density() const {
        if (concepts.empty()) return 0.0;
        
        // Approximate volume as product of coordinate ranges
        std::map<semantic_dimension, std::pair<double, double>> ranges;
        
        for (auto const& semantic_concept : concepts) {
            for (auto const& [dim, coord] : semantic_concept.get_coordinates()) {
                auto& [min_val, max_val] = ranges[dim];
                min_val = std::min(min_val, coord);
                max_val = std::max(max_val, coord);
            }
        }
        
        double volume = 1.0;
        for (auto const& [dim, range] : ranges) {
            double span = range.second - range.first;
            volume *= std::max(span, 0.1); // Avoid zero volume
        }
        
        return static_cast<double>(concepts.size()) / volume;
    }
    
private:
    void calculate_centroid() const {
        if (concepts.empty()) {
            centroid_cache = semantic_vector{};
            centroid_valid = true;
            return;
        }
        
        // Collect all dimensions
        std::unordered_set<semantic_dimension> all_dims;
        for (auto const& semantic_concept : concepts) {
            for (auto const& [dim, _] : semantic_concept.get_coordinates()) {
                all_dims.insert(dim);
            }
        }
        
        // Calculate average coordinates
        std::map<semantic_dimension, double> avg_coords;
        for (semantic_dimension dim : all_dims) {
            double sum = 0.0;
            for (auto const& semantic_concept : concepts) {
                sum += semantic_concept.get_coordinate(dim);
            }
            avg_coords[dim] = sum / concepts.size();
        }
        
        // Create synthetic content for centroid
        auto centroid_content = make_porter2_stem("centroid");
        centroid_cache = semantic_vector(centroid_content.value_or(porter2_stem{}), std::move(avg_coords));
        centroid_valid = true;
    }
};

/**
 * @brief Factory function for creating semantic vectors from text
 */
std::optional<semantic_vector> make_semantic_vector(std::string_view text);

/**
 * @brief Monoid composition for semantic vectors (concept fusion)
 * 
 * Combines two concepts by averaging their coordinates, representing
 * the emergence of new concepts from existing ones.
 */
semantic_vector operator*(semantic_vector const& lhs, semantic_vector const& rhs) {
    // Fuse content (simplified - could use more sophisticated combination)
    auto combined_content = lhs.content() * rhs.content();
    
    // Combine coordinates by averaging
    std::unordered_set<semantic_dimension> all_dims;
    for (auto const& [dim, _] : lhs.get_coordinates()) all_dims.insert(dim);
    for (auto const& [dim, _] : rhs.get_coordinates()) all_dims.insert(dim);
    
    std::map<semantic_dimension, double> combined_coords;
    for (semantic_dimension dim : all_dims) {
        double avg = (lhs.get_coordinate(dim) + rhs.get_coordinate(dim)) * 0.5;
        combined_coords[dim] = avg;
    }
    
    return semantic_vector(combined_content, std::move(combined_coords));
}

/**
 * @brief Optional monadic composition
 */
std::optional<semantic_vector> operator*(std::optional<semantic_vector> const& lhs,
                                        std::optional<semantic_vector> const& rhs) {
    if (!lhs || !rhs) return std::nullopt;
    return *lhs * *rhs;
}

/**
 * @brief Choice operator - select vector with higher semantic magnitude
 */
semantic_vector operator|(semantic_vector const& lhs, semantic_vector const& rhs) {
    return lhs.magnitude() >= rhs.magnitude() ? lhs : rhs;
}

/**
 * @brief Choice operator for optionals
 */
std::optional<semantic_vector> operator|(std::optional<semantic_vector> const& lhs,
                                        std::optional<semantic_vector> const& rhs) {
    return lhs ? lhs : rhs;
}

/**
 * @brief Repetition operator - semantic amplification
 */
semantic_vector operator^(semantic_vector const& base, size_t count) {
    if (count == 0) return semantic_vector{};
    if (count == 1) return base;
    
    // Amplify semantic coordinates by repetition factor
    semantic_vector result = base;
    double amplification = std::log2(count + 1); // Logarithmic amplification
    
    std::map<semantic_dimension, double> amplified_coords;
    for (auto const& [dim, coord] : base.get_coordinates()) {
        amplified_coords[dim] = std::clamp(coord * amplification, -1.0, 1.0);
    }
    
    return semantic_vector(base.content() ^ count, std::move(amplified_coords));
}

/**
 * @brief Function application for semantic analysis
 */
template<typename F>
    requires std::invocable<F, semantic_vector>
auto operator%(semantic_vector const& vector, F const& function)
    -> decltype(function(vector)) {
    return function(vector);
}

template<typename F>
    requires std::invocable<F, semantic_vector>
auto operator%(std::optional<semantic_vector> const& maybe_vector, F const& function)
    -> std::optional<std::decay_t<decltype(function(*maybe_vector))>> {
    if (!maybe_vector) return std::nullopt;
    return function(*maybe_vector);
}

/**
 * @brief Cluster composition - merge two semantic clusters
 */
semantic_cluster operator*(semantic_cluster const& lhs, semantic_cluster const& rhs) {
    auto combined = lhs.get_concepts();
    auto rhs_concepts = rhs.get_concepts();
    combined.insert(combined.end(), rhs_concepts.begin(), rhs_concepts.end());
    return semantic_cluster(std::move(combined));
}

/**
 * @brief Cluster choice - select more coherent cluster
 */
semantic_cluster operator|(semantic_cluster const& lhs, semantic_cluster const& rhs) {
    return lhs.coherence() >= rhs.coherence() ? lhs : rhs;
}

/**
 * @brief Semantic topology analyzer with algebraic composition
 * 
 * Analyzes text for semantic structure and provides compositional
 * operations for building complex semantic analyses.
 */
class semantic_topology_analyzer {
public:
    using input_type = std::string_view;
    using output_type = semantic_vector;
    
    /**
     * @brief Analyze text for semantic vector
     */
    std::optional<semantic_vector> operator()(std::string_view text) const {
        return make_semantic_vector(text);
    }
    
    /**
     * @brief Iterator-based parsing for composition
     */
    template<typename Iterator>
    std::pair<Iterator, std::optional<semantic_vector>> parse(Iterator begin, Iterator end) const {
        std::string word;
        Iterator current = begin;
        
        // Extract word characters
        while (current != end && std::isalpha(*current)) {
            word += std::tolower(*current);
            ++current;
        }
        
        if (word.empty()) {
            return {current, std::nullopt};
        }
        
        auto vector = make_semantic_vector(word);
        return {current, vector};
    }
    
    /**
     * @brief Cluster analysis for multiple concepts
     */
    std::optional<semantic_cluster> analyze_cluster(std::vector<std::string_view> const& texts) const {
        std::vector<semantic_vector> vectors;
        
        for (auto text : texts) {
            auto vector = make_semantic_vector(text);
            if (vector) {
                vectors.push_back(*vector);
            }
        }
        
        return vectors.empty() ? std::nullopt : 
               std::make_optional(semantic_cluster(std::move(vectors)));
    }
};

/**
 * @brief Predefined semantic analysis functions
 */
namespace semantic_analysis {
    /**
     * @brief Calculate semantic bridge between two distant concepts
     */
    inline double semantic_bridge_strength(semantic_vector const& concept1, 
                                          semantic_vector const& concept2) {
        double distance = concept1.distance(concept2);
        double similarity = concept1.similarity(concept2);
        
        // Bridge strength inversely related to distance but positively to similarity
        return similarity / (1.0 + distance);
    }
    
    /**
     * @brief Find conceptual path between two semantic vectors
     */
    inline std::vector<semantic_vector> find_semantic_path(
        semantic_vector const& start,
        semantic_vector const& end,
        std::vector<semantic_vector> const& available_concepts) {
        
        // Simplified pathfinding - in practice would use A* or similar
        std::vector<semantic_vector> path;
        path.push_back(start);
        
        semantic_vector current = start;
        double target_distance = current.distance(end);
        
        while (target_distance > 0.1 && path.size() < 10) { // Prevent infinite loops
            semantic_vector best_next = current;
            double best_improvement = 0.0;
            
            for (auto const& semantic_concept : available_concepts) {
                double new_distance = semantic_concept.distance(end);
                double improvement = target_distance - new_distance;
                
                if (improvement > best_improvement) {
                    best_improvement = improvement;
                    best_next = semantic_concept;
                }
            }
            
            if (best_improvement <= 0.0) break; // No improvement found
            
            path.push_back(best_next);
            current = best_next;
            target_distance = current.distance(end);
        }
        
        if (target_distance <= 0.1) {
            path.push_back(end);
        }
        
        return path;
    }
    
    /**
     * @brief Calculate semantic field density around a concept
     */
    inline double semantic_field_density(semantic_vector const& center,
                                        std::vector<semantic_vector> const& field,
                                        double radius = 0.5) {
        size_t neighbors = std::count_if(field.begin(), field.end(),
            [&](auto const& semantic_concept) {
                return center.distance(semantic_concept) <= radius;
            });
        
        // Density normalized by circular area in semantic space
        double area = M_PI * radius * radius;
        return static_cast<double>(neighbors) / area;
    }
}

/**
 * @brief Implementation of semantic vector factory function
 * 
 * Creates semantic vectors using heuristic analysis of word properties.
 * In a production system, this would use machine learning embeddings.
 */
std::optional<semantic_vector> make_semantic_vector(std::string_view text) {
    if (text.empty()) return std::nullopt;
    
    // Create stemmed content
    auto stem = make_porter2_stem(text);
    if (!stem) return std::nullopt;
    
    std::string word = static_cast<std::string>(*stem);
    semantic_vector vector(*stem);
    
    // Heuristic coordinate assignment based on word properties
    // This is simplified - real implementation would use trained embeddings
    
    // Concreteness: longer words tend to be more abstract
    double concreteness = std::clamp(1.0 - (word.length() - 5.0) * 0.1, -1.0, 1.0);
    vector.set_coordinate(semantic_dimension::concreteness, concreteness);
    
    // Complexity: based on syllable count and unusual letter combinations
    double complexity = std::clamp((word.length() - 4.0) * 0.15, -1.0, 1.0);
    vector.set_coordinate(semantic_dimension::complexity, complexity);
    
    // Positivity: very basic sentiment based on common patterns
    double positivity = 0.0;
    if (word.find("good") != std::string::npos || word.find("great") != std::string::npos ||
        word.find("love") != std::string::npos || word.find("joy") != std::string::npos) {
        positivity = 0.8;
    } else if (word.find("bad") != std::string::npos || word.find("hate") != std::string::npos ||
               word.find("sad") != std::string::npos || word.find("pain") != std::string::npos) {
        positivity = -0.8;
    }
    vector.set_coordinate(semantic_dimension::positivity, positivity);
    
    // Agency: words ending in -er, -or suggest agency
    double agency = 0.0;
    if (word.ends_with("er") || word.ends_with("or") || word.ends_with("ist")) {
        agency = 0.6;
    } else if (word.ends_with("ed") || word.ends_with("ing")) {
        agency = 0.3; // Past or ongoing action
    }
    vector.set_coordinate(semantic_dimension::agency, agency);
    
    // Temporality: words with temporal markers
    double temporality = 0.0;
    if (word.find("time") != std::string::npos || word.find("when") != std::string::npos ||
        word.find("now") != std::string::npos || word.find("then") != std::string::npos) {
        temporality = 0.7;
    }
    vector.set_coordinate(semantic_dimension::temporality, temporality);
    
    // Intensity: words with intensity markers
    double intensity = 0.0;
    if (word.find("very") != std::string::npos || word.find("extreme") != std::string::npos ||
        word.find("intense") != std::string::npos || word.length() > 10) {
        intensity = 0.6;
    }
    vector.set_coordinate(semantic_dimension::intensity, intensity);
    
    // Formality: longer, Latinate words tend to be more formal
    double formality = std::clamp((word.length() - 6.0) * 0.12, -1.0, 1.0);
    vector.set_coordinate(semantic_dimension::formality, formality);
    
    return vector;
}

/**
 * @brief Stream output for semantic vectors
 */
std::ostream& operator<<(std::ostream& os, semantic_vector const& vector) {
    os << "Semantic[" << static_cast<std::string>(vector.content()) 
       << ", mag=" << std::fixed << std::setprecision(2) << vector.magnitude() << "]";
    return os;
}

} // namespace alga
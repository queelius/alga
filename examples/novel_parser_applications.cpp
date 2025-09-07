/**
 * @file novel_parser_applications.cpp
 * @brief Real-world applications of novel algebraic text processing parsers
 * 
 * Demonstrates practical uses for rhythmic meter analysis, semantic topology,
 * and narrative structure parsing in literary criticism, content analysis,
 * educational technology, and creative writing assistance.
 */

#include "parsers/rhythmic_meter.hpp"
#include "parsers/semantic_topology.hpp"
#include "parsers/narrative_algebra.hpp"
#include "parsers/algebraic_operators.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>

using namespace algebraic_parsers;

// ============================================================================
// Application 1: Automated Poetry Analysis and Classification
// ============================================================================

void demonstrate_poetry_analysis() {
    std::cout << "=== Automated Poetry Analysis ===\n\n";
    
    // Sample poems with different metrical patterns
    std::vector<std::pair<std::string, std::string>> poems = {
        {"Shakespearean Sonnet", "Shall I compare thee to a summer's day? Thou art more lovely and more temperate"},
        {"Blake's Tyger", "Tyger Tyger burning bright, In the forests of the night"},
        {"Free Verse", "The fog comes on little cat feet. It sits looking over harbor and city"},
        {"Hymn Meter", "Amazing grace how sweet the sound that saved a wretch like me"}
    };
    
    rhythmic_meter_analyzer rhythm_analyzer;
    
    std::cout << "Analyzing metrical patterns in poetry:\n";
    std::cout << std::string(60, '-') << "\n";
    
    for (auto const& [title, text] : poems) {
        auto pattern = rhythm_analyzer(text);
        
        if (pattern) {
            using namespace classical_meters;
            
            std::cout << std::left << std::setw(20) << title << ": ";
            std::cout << *pattern << "\n";
            std::cout << "  Stress Density: " << std::fixed << std::setprecision(2) 
                      << pattern->stress_density() << "\n";
            std::cout << "  Regularity:    " << std::fixed << std::setprecision(3)
                      << pattern->regularity_metric() << "\n";
            std::cout << "  Classification: " << classify_meter(*pattern) << "\n";
            
            // Demonstrate compositional analysis
            auto emphasized = *pattern ^ 2; // Double for emphasis
            std::cout << "  Emphasized:     " << emphasized << "\n";
            
            // Apply analytical function
            auto complexity_measure = [](rhythmic_pattern const& p) {
                return p.stress_density() * (1.0 + p.regularity_metric());
            };
            
            auto complexity = *pattern % complexity_measure;
            std::cout << "  Complexity:     " << std::fixed << std::setprecision(2)
                      << complexity << "\n\n";
        }
    }
    
    // Demonstrate meter comparison using choice operator
    std::cout << "Metrical Comparison Using Algebraic Choice:\n";
    std::cout << std::string(40, '-') << "\n";
    
    auto pattern1 = rhythm_analyzer(poems[0].second);
    auto pattern2 = rhythm_analyzer(poems[1].second);
    
    if (pattern1 && pattern2) {
        auto preferred = *pattern1 | *pattern2; // Choice based on stress density
        std::cout << "Higher stress density pattern: " << preferred << "\n";
        
        // Compose patterns for hybrid analysis
        auto combined = *pattern1 * *pattern2;
        std::cout << "Combined pattern: " << combined << "\n";
        std::cout << "Combined density: " << combined.stress_density() << "\n\n";
    }
}

// ============================================================================
// Application 2: Semantic Content Analysis for Document Classification
// ============================================================================

void demonstrate_semantic_analysis() {
    std::cout << "=== Semantic Content Analysis ===\n\n";
    
    // Sample texts from different domains
    std::vector<std::pair<std::string, std::string>> documents = {
        {"Philosophy", "existence meaning truth reality consciousness experience knowledge"},
        {"Science", "hypothesis experiment data analysis theory evidence research"},
        {"Literature", "character plot narrative theme symbolism metaphor imagery"},
        {"Technology", "algorithm software system data processing computation network"},
        {"Psychology", "behavior emotion cognition memory learning perception mind"}
    };
    
    semantic_topology_analyzer semantic_analyzer;
    
    std::cout << "Analyzing semantic topology of documents:\n";
    std::cout << std::string(70, '-') << "\n";
    
    std::vector<semantic_cluster> domain_clusters;
    
    for (auto const& [domain, text] : documents) {
        // Split text into concepts and analyze each
        std::vector<semantic_vector> concepts;
        std::istringstream iss(text);
        std::string word;
        
        while (iss >> word) {
            auto vector = semantic_analyzer(word);
            if (vector) {
                concepts.push_back(*vector);
            }
        }
        
        if (!concepts.empty()) {
            semantic_cluster cluster(concepts);
            domain_clusters.push_back(cluster);
            
            std::cout << std::left << std::setw(15) << domain << ": ";
            std::cout << concepts.size() << " concepts\n";
            std::cout << "  Coherence: " << std::fixed << std::setprecision(3)
                      << cluster.coherence() << "\n";
            std::cout << "  Density:   " << std::fixed << std::setprecision(3)
                      << cluster.density() << "\n";
            
            auto centroid = cluster.centroid();
            std::cout << "  Centroid:  " << centroid << "\n";
            
            // Demonstrate semantic composition
            if (concepts.size() >= 2) {
                auto fusion = concepts[0] * concepts[1]; // Concept fusion
                std::cout << "  Fusion:    " << fusion << "\n";
            }
            
            std::cout << "\n";
        }
    }
    
    // Cross-domain semantic analysis
    std::cout << "Cross-Domain Semantic Relationships:\n";
    std::cout << std::string(40, '-') << "\n";
    
    for (size_t i = 0; i < domain_clusters.size(); ++i) {
        for (size_t j = i + 1; j < domain_clusters.size(); ++j) {
            auto& cluster1 = domain_clusters[i];
            auto& cluster2 = domain_clusters[j];
            
            // Calculate semantic distance between domain centroids
            double distance = cluster1.centroid().distance(cluster2.centroid());
            double similarity = cluster1.centroid().similarity(cluster2.centroid());
            
            std::cout << documents[i].first << " ↔ " << documents[j].first << ": ";
            std::cout << "dist=" << std::fixed << std::setprecision(2) << distance
                      << ", sim=" << std::fixed << std::setprecision(2) << similarity << "\n";
        }
    }
    
    // Demonstrate semantic field analysis
    std::cout << "\nSemantic Field Density Analysis:\n";
    std::cout << std::string(35, '-') << "\n";
    
    using namespace semantic_analysis;
    
    if (domain_clusters.size() >= 2) {
        auto& phil_cluster = domain_clusters[0]; // Philosophy
        auto& sci_cluster = domain_clusters[1];  // Science
        
        auto phil_concepts = phil_cluster.get_concepts();
        auto sci_concepts = sci_cluster.get_concepts();
        
        if (!phil_concepts.empty() && !sci_concepts.empty()) {
            double field_density = semantic_field_density(
                phil_concepts[0], sci_concepts, 0.8
            );
            
            std::cout << "Philosophy concept in Science field: "
                      << std::fixed << std::setprecision(3) << field_density << "\n";
            
            // Find semantic bridge
            double bridge_strength = semantic_bridge_strength(
                phil_concepts[0], sci_concepts[0]
            );
            
            std::cout << "Interdisciplinary bridge strength: "
                      << std::fixed << std::setprecision(3) << bridge_strength << "\n\n";
        }
    }
}

// ============================================================================
// Application 3: Narrative Structure Analysis for Literary Criticism
// ============================================================================

void demonstrate_narrative_analysis() {
    std::cout << "=== Narrative Structure Analysis ===\n\n";
    
    // Sample story excerpts representing different narrative elements
    std::vector<std::string> story_segments = {
        "In a small village nestled between rolling hills lived a young blacksmith named Elena.",
        "One morning, a mysterious stranger arrived carrying an ancient sword that glowed with inner fire.",
        "Elena discovered that the sword belonged to her grandfather, a legendary warrior thought to be dead.",
        "The stranger revealed that dark forces were gathering to destroy the village and everyone in it.",
        "Elena struggled to master the sword's power while battling her own fears and self-doubt.",
        "In the climactic battle, Elena channeled generations of her family's courage to defeat the darkness.",
        "The village was saved, and Elena emerged as a true warrior, worthy of her grandfather's legacy.",
        "Peace returned to the land, and Elena became the village's protector for generations to come."
    };
    
    narrative_algebra_analyzer narrative_analyzer;
    
    std::cout << "Analyzing narrative structure:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Convert strings to string_views for analysis
    std::vector<std::string_view> story_segments_view;
    for (auto const& segment : story_segments) {
        story_segments_view.push_back(segment);
    }
    
    // Analyze overall story structure
    auto story_structure = narrative_analyzer.analyze_structure(story_segments_view);
    
    if (story_structure) {
        std::cout << "Story Elements: " << story_structure->size() << "\n";
        
        auto tension_curve = story_structure->tension_curve();
        std::cout << "Tension Curve: ";
        for (size_t i = 0; i < tension_curve.size(); ++i) {
            std::cout << std::fixed << std::setprecision(1) << tension_curve[i];
            if (i < tension_curve.size() - 1) std::cout << " → ";
        }
        std::cout << "\n";
        
        std::cout << "Peak Tension: " << std::fixed << std::setprecision(2)
                  << story_structure->peak_tension() << "\n";
        std::cout << "Climax Position: " << story_structure->climax_position() + 1
                  << " of " << story_structure->size() << "\n";
        std::cout << "Narrative Coherence: " << std::fixed << std::setprecision(3)
                  << story_structure->coherence() << "\n";
        std::cout << "Pacing Variance: " << std::fixed << std::setprecision(3)
                  << story_structure->pacing_variance() << "\n";
        
        using namespace narrative_patterns;
        std::string classification = classify_narrative(*story_structure);
        std::cout << "Structure Type: " << classification << "\n";
        
        auto character_dev = story_structure->character_development();
        if (!character_dev.empty()) {
            std::cout << "Character Development Tracked: " << character_dev.size()
                      << " relationships\n";
        }
        
        std::cout << "Thematic Density: " << std::fixed << std::setprecision(2)
                  << story_structure->thematic_density() << "\n\n";
    }
    
    // Analyze individual narrative elements
    std::cout << "Individual Element Analysis:\n";
    std::cout << std::string(30, '-') << "\n";
    
    std::vector<std::string> element_types = {
        "Exposition", "Inciting Incident", "Character Revelation", "Rising Action",
        "Character Development", "Climax", "Resolution", "Denouement"
    };
    
    for (size_t i = 0; i < std::min(story_segments.size(), element_types.size()); ++i) {
        auto element = narrative_analyzer(story_segments[i]);
        
        if (element) {
            std::cout << std::left << std::setw(20) << element_types[i] << ": ";
            std::cout << "T=" << std::fixed << std::setprecision(2)
                      << element->get_tension().magnitude()
                      << ", D=" << std::fixed << std::setprecision(1)
                      << element->get_duration()
                      << ", C=" << std::fixed << std::setprecision(2)
                      << element->get_causal_weight() << "\n";
        }
    }
    
    // Demonstrate narrative composition
    std::cout << "\nNarrative Composition Examples:\n";
    std::cout << std::string(35, '-') << "\n";
    
    auto exposition = narrative_analyzer(story_segments[0]);
    auto climax = narrative_analyzer(story_segments[5]);
    
    if (exposition && climax) {
        // Sequential composition
        auto sequence = *exposition * *climax;
        std::cout << "Exposition + Climax: T="
                  << std::fixed << std::setprecision(2) << sequence.get_tension().magnitude()
                  << ", D=" << std::fixed << std::setprecision(1) << sequence.get_duration() << "\n";
        
        // Choice operation (higher tension wins)
        auto choice = *exposition | *climax;
        std::cout << "Exposition | Climax: T="
                  << std::fixed << std::setprecision(2) << choice.get_tension().magnitude() << "\n";
        
        // Repetition for emphasis
        auto emphasized = *climax ^ 2;
        std::cout << "Climax ^ 2: T="
                  << std::fixed << std::setprecision(2) << emphasized.get_tension().magnitude() << "\n";
    }
    
    // Compare with classical patterns
    std::cout << "\nComparison with Classical Patterns:\n";
    std::cout << std::string(40, '-') << "\n";
    
    using namespace narrative_patterns;
    
    auto three_act = three_act_structure();
    auto heros_journey_pattern = heros_journey();
    auto tragedy = tragedy_pattern();
    
    std::cout << "Three-Act Structure: " << classify_narrative(three_act) << "\n";
    std::cout << "Hero's Journey: " << classify_narrative(heros_journey_pattern) << "\n";
    std::cout << "Tragedy Pattern: " << classify_narrative(tragedy) << "\n\n";
}

// ============================================================================
// Application 4: Multi-Dimensional Literary Quality Assessment
// ============================================================================

void demonstrate_quality_assessment() {
    std::cout << "=== Multi-Dimensional Literary Quality Assessment ===\n\n";
    
    std::vector<std::pair<std::string, std::string>> text_samples = {
        {"High Literature", "The luminous cathedral of consciousness expanded beyond temporal boundaries, embracing infinite possibilities of human experience and transcendent understanding."},
        {"Technical Writing", "The algorithm processes input data through multiple stages of validation, transformation, and optimization to produce the desired output format."},
        {"Popular Fiction", "Sarah ran through the dark forest as the monster chased her, branches tearing at her clothes and fear pounding in her heart."},
        {"Poetry", "Gentle moonlight whispers secrets to the silent stones, while ancient oaks remember forgotten dreams of sleeping earth."}
    };
    
    rhythmic_meter_analyzer rhythm_analyzer;
    semantic_topology_analyzer semantic_analyzer;
    narrative_algebra_analyzer narrative_analyzer;
    
    std::cout << "Comprehensive quality assessment:\n";
    std::cout << std::string(80, '=') << "\n";
    
    for (auto const& [category, text] : text_samples) {
        std::cout << "\n" << category << ":\n";
        std::cout << "\"" << text.substr(0, 60) << (text.length() > 60 ? "..." : "") << "\"\n";
        std::cout << std::string(60, '-') << "\n";
        
        // Rhythmic analysis
        auto rhythm = rhythm_analyzer(text);
        double rhythmic_quality = 0.0;
        if (rhythm) {
            double density = rhythm->stress_density();
            double regularity = 1.0 / (1.0 + rhythm->regularity_metric());
            rhythmic_quality = (density + regularity) * 0.5;
            
            std::cout << "Rhythmic Quality:  " << std::fixed << std::setprecision(2)
                      << rhythmic_quality << " (density=" << density
                      << ", regularity=" << regularity << ")\n";
        }
        
        // Semantic analysis
        auto semantics = semantic_analyzer(text);
        double semantic_quality = 0.0;
        if (semantics) {
            semantic_quality = std::min(1.0, semantics->magnitude() * 0.5);
            
            std::cout << "Semantic Density:  " << std::fixed << std::setprecision(2)
                      << semantic_quality << " (magnitude=" << semantics->magnitude() << ")\n";
        }
        
        // Narrative analysis
        auto narrative = narrative_analyzer(text);
        double narrative_quality = 0.0;
        if (narrative) {
            double tension = narrative->get_tension().magnitude();
            double causal_weight = narrative->get_causal_weight();
            narrative_quality = (tension + causal_weight) * 0.5;
            
            std::cout << "Narrative Strength: " << std::fixed << std::setprecision(2)
                      << narrative_quality << " (tension=" << tension
                      << ", causality=" << causal_weight << ")\n";
        }
        
        // Composite quality score using algebraic composition
        double composite_quality = 0.0;
        if (rhythm && semantics && narrative) {
            // Weighted combination that demonstrates algebraic thinking
            composite_quality = (rhythmic_quality * 0.3 +
                               semantic_quality * 0.4 +
                               narrative_quality * 0.3);
            
            std::cout << "Composite Quality:  " << std::fixed << std::setprecision(2)
                      << composite_quality << "\n";
            
            // Quality classification
            std::string quality_class;
            if (composite_quality >= 0.8) quality_class = "Exceptional";
            else if (composite_quality >= 0.6) quality_class = "High";
            else if (composite_quality >= 0.4) quality_class = "Moderate";
            else if (composite_quality >= 0.2) quality_class = "Basic";
            else quality_class = "Poor";
            
            std::cout << "Quality Class:      " << quality_class << "\n";
        }
    }
}

// ============================================================================
// Application 5: Creative Writing Assistant
// ============================================================================

void demonstrate_writing_assistant() {
    std::cout << "\n=== Creative Writing Assistant ===\n\n";
    
    // Simulate an iterative writing process with algebraic composition
    std::cout << "Iterative Story Development:\n";
    std::cout << std::string(30, '-') << "\n";
    
    narrative_algebra_analyzer narrative_analyzer;
    
    // Start with a basic story seed
    std::string seed = "A young artist discovered a magical paintbrush in her grandmother's attic.";
    std::cout << "Story Seed: \"" << seed << "\"\n\n";
    
    auto initial_element = narrative_analyzer(seed);
    if (!initial_element) {
        std::cout << "Error: Could not analyze initial story seed.\n";
        return;
    }
    
    narrative_structure developing_story{*initial_element};
    
    // Suggest developments based on narrative analysis
    std::vector<std::string> potential_developments = {
        "The paintbrush created living creatures from whatever she painted on canvas.",
        "She realized the brush belonged to a famous artist who had mysteriously disappeared.",
        "Using the brush, she accidentally opened a portal to a world of living art.",
        "The magical paintings began changing reality around her in unexpected ways."
    };
    
    std::cout << "Development Suggestions (with algebraic analysis):\n";
    for (size_t i = 0; i < potential_developments.size(); ++i) {
        auto dev_element = narrative_analyzer(potential_developments[i]);
        if (dev_element) {
            // Analyze compatibility with current story
            double compatibility = initial_element->compatibility(*dev_element);
            
            // Project narrative after adding this development
            auto extended_story = developing_story * narrative_structure{*dev_element};
            
            std::cout << (i + 1) << ". \"" << potential_developments[i] << "\"\n";
            std::cout << "   Compatibility: " << std::fixed << std::setprecision(2)
                      << compatibility << "\n";
            std::cout << "   New Tension:   " << std::fixed << std::setprecision(2)
                      << dev_element->get_tension().magnitude() << "\n";
            std::cout << "   Story Coherence: " << std::fixed << std::setprecision(2)
                      << extended_story.coherence() << "\n\n";
        }
    }
    
    // Demonstrate rhythm-aware dialogue generation
    std::cout << "Rhythm-Aware Dialogue Suggestions:\n";
    std::cout << std::string(35, '-') << "\n";
    
    rhythmic_meter_analyzer rhythm_analyzer;
    
    std::vector<std::string> dialogue_options = {
        "I can't believe what I'm seeing right now!",
        "This is absolutely impossible to comprehend.",
        "What magic is this that flows from brush to world?",
        "The paintings... they're alive somehow."
    };
    
    for (auto const& dialogue : dialogue_options) {
        auto rhythm = rhythm_analyzer(dialogue);
        if (rhythm) {
            std::cout << "\"" << dialogue << "\"\n";
            std::cout << "  Pattern: " << *rhythm << "\n";
            std::cout << "  Flow Score: " << std::fixed << std::setprecision(2)
                      << (1.0 - rhythm->regularity_metric()) << "\n\n";
        }
    }
    
    // Semantic consistency checking
    std::cout << "Semantic Consistency Analysis:\n";
    std::cout << std::string(30, '-') << "\n";
    
    semantic_topology_analyzer semantic_analyzer;
    
    std::vector<std::string> thematic_elements = {
        "magic", "art", "creativity", "discovery", "transformation"
    };
    
    std::vector<semantic_vector> theme_vectors;
    for (auto const& theme : thematic_elements) {
        auto vector = semantic_analyzer(theme);
        if (vector) {
            theme_vectors.push_back(*vector);
        }
    }
    
    if (!theme_vectors.empty()) {
        semantic_cluster story_themes(theme_vectors);
        
        std::cout << "Thematic Coherence: " << std::fixed << std::setprecision(2)
                  << story_themes.coherence() << "\n";
        std::cout << "Thematic Density: " << std::fixed << std::setprecision(2)
                  << story_themes.density() << "\n";
        std::cout << "Central Theme: " << story_themes.centroid() << "\n\n";
        
        // Suggest thematically consistent elements
        std::cout << "Suggested Thematic Elements:\n";
        
        using namespace semantic_analysis;
        
        std::vector<std::string> candidates = {
            "inspiration", "color", "reality", "imagination", "power"
        };
        
        for (auto const& candidate : candidates) {
            auto candidate_vector = semantic_analyzer(candidate);
            if (candidate_vector) {
                double field_density = semantic_field_density(
                    *candidate_vector, theme_vectors, 0.6
                );
                
                std::cout << "  " << candidate << ": density="
                          << std::fixed << std::setprecision(2) << field_density;
                if (field_density > 0.3) std::cout << " ✓ (good fit)";
                std::cout << "\n";
            }
        }
    }
}

// ============================================================================
// Main Application Runner
// ============================================================================

int main() {
    std::cout << "🎭 Algebraic Text Processing: Novel Parser Applications\n";
    std::cout << std::string(80, '=') << "\n";
    std::cout << "Demonstrating real-world applications of mathematical text analysis\n\n";
    
    try {
        demonstrate_poetry_analysis();
        std::cout << "\n" << std::string(80, '=') << "\n";
        
        demonstrate_semantic_analysis();
        std::cout << "\n" << std::string(80, '=') << "\n";
        
        demonstrate_narrative_analysis();
        std::cout << "\n" << std::string(80, '=') << "\n";
        
        demonstrate_quality_assessment();
        std::cout << "\n" << std::string(80, '=') << "\n";
        
        demonstrate_writing_assistant();
        
        std::cout << "\n" << std::string(80, '=') << "\n";
        std::cout << "🚀 Conclusion: Algebraic composition transforms text processing\n";
        std::cout << "   from crude pattern matching into sophisticated mathematical\n";
        std::cout << "   analysis that reveals the hidden mathematical structures\n";
        std::cout << "   embedded in human language and literature.\n\n";
        std::cout << "   Applications span: literary criticism, content analysis,\n";
        std::cout << "   educational assessment, creative writing assistance,\n";
        std::cout << "   and cross-cultural narrative analysis.\n\n";
        
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
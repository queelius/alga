#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <optional>
#include <numeric>

namespace alga {
namespace statistics {

/**
 * @brief Frequency counter for any hashable type
 *
 * Counts occurrences of elements and provides frequency analysis.
 */
template<typename T>
class FrequencyCounter {
private:
    std::unordered_map<T, size_t> counts;
    size_t total_count;

public:
    FrequencyCounter() : total_count(0) {}

    /**
     * @brief Add an element to the frequency counter
     */
    void add(const T& element) {
        ++counts[element];
        ++total_count;
    }

    /**
     * @brief Add multiple elements
     */
    void add_all(const std::vector<T>& elements) {
        for (const auto& elem : elements) {
            add(elem);
        }
    }

    /**
     * @brief Get count for a specific element
     */
    size_t count(const T& element) const {
        auto it = counts.find(element);
        return it != counts.end() ? it->second : 0;
    }

    /**
     * @brief Get total number of elements counted
     */
    size_t total() const {
        return total_count;
    }

    /**
     * @brief Get number of unique elements
     */
    size_t unique_count() const {
        return counts.size();
    }

    /**
     * @brief Get frequency (proportion) of an element
     */
    double frequency(const T& element) const {
        if (total_count == 0) return 0.0;
        return static_cast<double>(count(element)) / total_count;
    }

    /**
     * @brief Get all elements sorted by frequency (descending)
     */
    std::vector<std::pair<T, size_t>> most_common() const {
        std::vector<std::pair<T, size_t>> sorted(counts.begin(), counts.end());
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        return sorted;
    }

    /**
     * @brief Get top N most common elements
     */
    std::vector<std::pair<T, size_t>> top_n(size_t n) const {
        auto all = most_common();
        if (all.size() <= n) return all;
        return std::vector<std::pair<T, size_t>>(all.begin(), all.begin() + n);
    }

    /**
     * @brief Get all elements sorted by frequency (ascending)
     */
    std::vector<std::pair<T, size_t>> least_common() const {
        std::vector<std::pair<T, size_t>> sorted(counts.begin(), counts.end());
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto& a, const auto& b) { return a.second < b.second; });
        return sorted;
    }

    /**
     * @brief Get the mode (most common element)
     */
    std::optional<T> mode() const {
        if (counts.empty()) return std::nullopt;
        auto common = most_common();
        return common[0].first;
    }

    /**
     * @brief Get all counts as a map
     */
    const std::unordered_map<T, size_t>& get_counts() const {
        return counts;
    }

    /**
     * @brief Clear all counts
     */
    void clear() {
        counts.clear();
        total_count = 0;
    }
};

/**
 * @brief Calculate Shannon entropy
 *
 * Measures information content / diversity of a distribution.
 * Higher entropy = more diverse / unpredictable.
 */
template<typename T>
double shannon_entropy(const FrequencyCounter<T>& counter) {
    if (counter.total() == 0) return 0.0;

    double entropy = 0.0;
    for (const auto& [element, count] : counter.get_counts()) {
        double p = static_cast<double>(count) / counter.total();
        if (p > 0) {
            entropy -= p * std::log2(p);
        }
    }

    return entropy;
}

/**
 * @brief Calculate normalized Shannon entropy (0-1 range)
 *
 * Normalizes by the maximum possible entropy for the given number of unique elements.
 */
template<typename T>
double normalized_entropy(const FrequencyCounter<T>& counter) {
    if (counter.unique_count() <= 1) return 0.0;

    double max_entropy = std::log2(counter.unique_count());
    double actual_entropy = shannon_entropy(counter);

    return actual_entropy / max_entropy;
}

/**
 * @brief Simpson's diversity index
 *
 * Probability that two randomly selected elements are different.
 * Range: 0 (no diversity) to 1 (maximum diversity).
 */
template<typename T>
double simpson_diversity(const FrequencyCounter<T>& counter) {
    if (counter.total() <= 1) return 0.0;

    double sum_squared_proportions = 0.0;
    for (const auto& [element, count] : counter.get_counts()) {
        double p = static_cast<double>(count) / counter.total();
        sum_squared_proportions += p * p;
    }

    return 1.0 - sum_squared_proportions;
}

/**
 * @brief Basic statistical summary for numeric types
 */
template<typename T>
struct NumericSummary {
    size_t count;
    T min;
    T max;
    double mean;
    double variance;
    double std_dev;
    T median;

    NumericSummary()
        : count(0), min(T{}), max(T{}),
          mean(0.0), variance(0.0), std_dev(0.0), median(T{}) {}
};

/**
 * @brief Calculate summary statistics for numeric data
 */
template<typename T>
std::optional<NumericSummary<T>> numeric_summary(const std::vector<T>& data) {
    if (data.empty()) return std::nullopt;

    NumericSummary<T> summary;
    summary.count = data.size();

    // Min and max
    summary.min = *std::min_element(data.begin(), data.end());
    summary.max = *std::max_element(data.begin(), data.end());

    // Mean
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    summary.mean = sum / data.size();

    // Variance and standard deviation
    double sq_sum = 0.0;
    for (const auto& value : data) {
        double diff = value - summary.mean;
        sq_sum += diff * diff;
    }
    summary.variance = sq_sum / data.size();
    summary.std_dev = std::sqrt(summary.variance);

    // Median (requires sorted copy)
    std::vector<T> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    if (sorted_data.size() % 2 == 0) {
        size_t mid = sorted_data.size() / 2;
        summary.median = (sorted_data[mid - 1] + sorted_data[mid]) / 2;
    } else {
        summary.median = sorted_data[sorted_data.size() / 2];
    }

    return summary;
}

/**
 * @brief Gini coefficient (measure of inequality)
 *
 * Range: 0 (perfect equality) to 1 (maximum inequality).
 * Useful for analyzing distribution of frequencies.
 */
template<typename T>
double gini_coefficient(const FrequencyCounter<T>& counter) {
    if (counter.total() == 0) return 0.0;

    // Get sorted counts
    std::vector<size_t> counts;
    for (const auto& [element, count] : counter.get_counts()) {
        counts.push_back(count);
    }
    std::sort(counts.begin(), counts.end());

    double sum_of_products = 0.0;
    for (size_t i = 0; i < counts.size(); ++i) {
        sum_of_products += (i + 1) * counts[i];
    }

    double n = counts.size();
    double sum_of_counts = counter.total();

    if (sum_of_counts == 0) return 0.0;

    return (2.0 * sum_of_products) / (n * sum_of_counts) - (n + 1) / n;
}

/**
 * @brief Type token ratio (vocabulary richness)
 *
 * Ratio of unique elements to total elements.
 * Range: 0 to 1, higher = more diverse vocabulary.
 */
template<typename T>
double type_token_ratio(const FrequencyCounter<T>& counter) {
    if (counter.total() == 0) return 0.0;
    return static_cast<double>(counter.unique_count()) / counter.total();
}

/**
 * @brief Hapax legomena count
 *
 * Number of elements that appear exactly once.
 * Useful for assessing vocabulary richness.
 */
template<typename T>
size_t hapax_legomena_count(const FrequencyCounter<T>& counter) {
    size_t hapax_count = 0;
    for (const auto& [element, count] : counter.get_counts()) {
        if (count == 1) {
            ++hapax_count;
        }
    }
    return hapax_count;
}

/**
 * @brief Dis legomena count
 *
 * Number of elements that appear exactly twice.
 */
template<typename T>
size_t dis_legomena_count(const FrequencyCounter<T>& counter) {
    size_t dis_count = 0;
    for (const auto& [element, count] : counter.get_counts()) {
        if (count == 2) {
            ++dis_count;
        }
    }
    return dis_count;
}

/**
 * @brief Statistical analyzer - combines multiple metrics
 */
template<typename T>
struct DistributionAnalysis {
    size_t total_elements;
    size_t unique_elements;
    double shannon_entropy;
    double normalized_entropy;
    double simpson_diversity;
    double gini_coefficient;
    double type_token_ratio;
    size_t hapax_count;
    size_t dis_count;
    std::vector<std::pair<T, size_t>> top_10;

    DistributionAnalysis()
        : total_elements(0), unique_elements(0),
          shannon_entropy(0.0), normalized_entropy(0.0),
          simpson_diversity(0.0), gini_coefficient(0.0),
          type_token_ratio(0.0), hapax_count(0), dis_count(0) {}
};

/**
 * @brief Comprehensive distribution analysis
 */
template<typename T>
DistributionAnalysis<T> analyze_distribution(const FrequencyCounter<T>& counter) {
    DistributionAnalysis<T> analysis;

    analysis.total_elements = counter.total();
    analysis.unique_elements = counter.unique_count();
    analysis.shannon_entropy = shannon_entropy(counter);
    analysis.normalized_entropy = normalized_entropy(counter);
    analysis.simpson_diversity = simpson_diversity(counter);
    analysis.gini_coefficient = gini_coefficient(counter);
    analysis.type_token_ratio = type_token_ratio(counter);
    analysis.hapax_count = hapax_legomena_count(counter);
    analysis.dis_count = dis_legomena_count(counter);
    analysis.top_10 = counter.top_n(10);

    return analysis;
}

/**
 * @brief Create a frequency counter from a vector
 */
template<typename T>
FrequencyCounter<T> make_frequency_counter(const std::vector<T>& elements) {
    FrequencyCounter<T> counter;
    counter.add_all(elements);
    return counter;
}

} // namespace statistics
} // namespace alga

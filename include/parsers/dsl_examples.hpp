#pragma once

#include "monadic_combinators.hpp"
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>

namespace algebraic_parsers {
namespace dsl {

/**
 * @brief Natural Language Processing Pipeline
 * 
 * Demonstrates complex parser composition for text analysis.
 * Shows how our algebraic types compose naturally for real-world tasks.
 */
class NLPPipeline {
public:
    porter2_stemmer stemmer;
    
    /**
     * @brief Parse and analyze a sentence into structured linguistic data
     */
    struct SentenceAnalysis {
        std::vector<lc_alpha> words;           // Original words
        std::vector<porter2_stem> stems;       // Stemmed forms
        std::vector<bigram_stem> bigrams;      // Word pairs
        std::vector<trigram_stem> trigrams;    // Word triples
        size_t word_count;
        size_t unique_stems;
        
        explicit operator std::string() const {
            std::ostringstream oss;
            oss << "Words: " << word_count 
                << ", Unique stems: " << unique_stems
                << ", Bigrams: " << bigrams.size()
                << ", Trigrams: " << trigrams.size();
            return oss.str();
        }
    };
    
    std::optional<SentenceAnalysis> analyze_sentence(std::string_view input) const {
        // Step 1: Extract words using monadic composition
        auto words = extract_words(input);
        if (!words || words->empty()) {
            return {};
        }
        
        // Step 2: Stem all words using monadic bind
        auto stems = words >>= [this](auto const& word_list) {
            return stem_all_words(word_list);
        };
        
        if (!stems) {
            return {};
        }
        
        // Step 3: Generate n-grams using algebraic composition
        auto bigrams = generate_bigrams(*stems);
        auto trigrams = generate_trigrams(*stems);
        
        // Step 4: Calculate statistics
        std::set<porter2_stem> unique_stem_set(stems->begin(), stems->end());
        
        return SentenceAnalysis{
            *words,
            *stems,
            bigrams,
            trigrams,
            words->size(),
            unique_stem_set.size()
        };
    }
    
    /**
     * @brief Parallel processing of multiple sentences
     */
    auto analyze_corpus(std::vector<std::string> const& sentences) const {
        using namespace monadic;
        
        // Create parallel parser for all sentences
        return std::async(std::launch::async, [&]() {
            std::vector<std::optional<SentenceAnalysis>> results;
            results.reserve(sentences.size());
            
            // Process all sentences in parallel
            std::vector<std::future<std::optional<SentenceAnalysis>>> futures;
            for (auto const& sentence : sentences) {
                futures.emplace_back(std::async(std::launch::async, [&, sentence]() {
                    return analyze_sentence(sentence);
                }));
            }
            
            // Collect results
            for (auto& future : futures) {
                results.push_back(future.get());
            }
            
            return results;
        });
    }
    
private:
    std::optional<std::vector<lc_alpha>> extract_words(std::string_view input) const {
        std::vector<lc_alpha> words;
        std::regex word_regex(R"(\b[a-zA-Z]+\b)");
        
        auto begin = std::sregex_iterator(input.begin(), input.end(), word_regex);
        auto end = std::sregex_iterator();
        
        for (auto it = begin; it != end; ++it) {
            auto word = make_lc_alpha(it->str());
            if (!word) {
                continue; // Skip invalid words
            }
            words.push_back(*word);
        }
        
        return words.empty() ? std::nullopt : std::make_optional(words);
    }
    
    std::optional<std::vector<porter2_stem>> stem_all_words(std::vector<lc_alpha> const& words) const {
        std::vector<porter2_stem> stems;
        stems.reserve(words.size());
        
        for (auto const& word : words) {
            auto stem = stemmer(word);
            if (!stem) {
                return {}; // Propagate failure
            }
            stems.push_back(*stem);
        }
        
        return stems;
    }
    
    std::vector<bigram_stem> generate_bigrams(std::vector<porter2_stem> const& stems) const {
        std::vector<bigram_stem> bigrams;
        if (stems.size() < 2) return bigrams;
        
        for (size_t i = 0; i < stems.size() - 1; ++i) {
            bigrams.push_back(make_bigram(stems[i], stems[i + 1]));
        }
        
        return bigrams;
    }
    
    std::vector<trigram_stem> generate_trigrams(std::vector<porter2_stem> const& stems) const {
        std::vector<trigram_stem> trigrams;
        if (stems.size() < 3) return trigrams;
        
        for (size_t i = 0; i < stems.size() - 2; ++i) {
            trigrams.push_back(make_trigram(stems[i], stems[i + 1], stems[i + 2]));
        }
        
        return trigrams;
    }
};

/**
 * @brief Configuration File Parser
 * 
 * Demonstrates recursive descent parsing with algebraic composition.
 */
class ConfigParser {
public:
    struct ConfigValue {
        std::string key;
        std::string value;
        
        bool operator==(ConfigValue const& other) const {
            return key == other.key && value == other.value;
        }
    };
    
    struct ConfigSection {
        std::string name;
        std::vector<ConfigValue> values;
        
        bool operator==(ConfigSection const& other) const {
            return name == other.name && values == other.values;
        }
    };
    
    struct ConfigFile {
        std::vector<ConfigSection> sections;
        
        explicit operator std::string() const {
            std::ostringstream oss;
            for (auto const& section : sections) {
                oss << "[" << section.name << "]\n";
                for (auto const& value : section.values) {
                    oss << value.key << " = " << value.value << "\n";
                }
                oss << "\n";
            }
            return oss.str();
        }
    };
    
    std::optional<ConfigFile> parse_config(std::string_view input) const {
        using namespace monadic;
        
        // Tokenize the input
        auto tokens = tokenize(input);
        if (!tokens) {
            return {};
        }
        
        // Parse sections using monadic composition
        auto sections = tokens >>= [this](auto const& token_list) {
            return parse_sections(token_list);
        };
        
        if (!sections) {
            return {};
        }
        
        return ConfigFile{*sections};
    }
    
private:
    enum class TokenType { Section, Key, Value, Comment, Empty };
    
    struct Token {
        TokenType type;
        std::string content;
    };
    
    std::optional<std::vector<Token>> tokenize(std::string_view input) const {
        std::vector<Token> tokens;
        std::istringstream iss(std::string(input));
        std::string line;
        
        while (std::getline(iss, line)) {
            auto token = parse_line(line);
            if (token) {
                tokens.push_back(*token);
            }
        }
        
        return tokens.empty() ? std::nullopt : std::make_optional(tokens);
    }
    
    std::optional<Token> parse_line(std::string const& line) const {
        // Remove leading/trailing whitespace
        auto trimmed = trim(line);
        
        if (trimmed.empty()) {
            return Token{TokenType::Empty, ""};
        }
        
        if (trimmed[0] == '#') {
            return Token{TokenType::Comment, trimmed};
        }
        
        if (trimmed[0] == '[' && trimmed.back() == ']') {
            auto section_name = trimmed.substr(1, trimmed.length() - 2);
            return Token{TokenType::Section, section_name};
        }
        
        auto eq_pos = trimmed.find('=');
        if (eq_pos != std::string::npos) {
            auto key = trim(trimmed.substr(0, eq_pos));
            auto value = trim(trimmed.substr(eq_pos + 1));
            // For simplicity, encode key-value in content
            return Token{TokenType::Key, key + "=" + value};
        }
        
        return {}; // Invalid line
    }
    
    std::optional<std::vector<ConfigSection>> parse_sections(std::vector<Token> const& tokens) const {
        std::vector<ConfigSection> sections;
        ConfigSection current_section;
        bool in_section = false;
        
        for (auto const& token : tokens) {
            switch (token.type) {
                case TokenType::Section:
                    if (in_section) {
                        sections.push_back(std::move(current_section));
                    }
                    current_section = ConfigSection{token.content, {}};
                    in_section = true;
                    break;
                    
                case TokenType::Key:
                    if (in_section) {
                        auto kv = parse_key_value(token.content);
                        if (kv) {
                            current_section.values.push_back(*kv);
                        }
                    }
                    break;
                    
                case TokenType::Comment:
                case TokenType::Empty:
                    break; // Skip comments and empty lines
            }
        }
        
        if (in_section) {
            sections.push_back(std::move(current_section));
        }
        
        return sections.empty() ? std::nullopt : std::make_optional(sections);
    }
    
    std::optional<ConfigValue> parse_key_value(std::string const& content) const {
        auto eq_pos = content.find('=');
        if (eq_pos == std::string::npos) {
            return {};
        }
        
        auto key = content.substr(0, eq_pos);
        auto value = content.substr(eq_pos + 1);
        
        return ConfigValue{key, value};
    }
    
    std::string trim(std::string const& str) const {
        auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            return "";
        }
        auto end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
};

/**
 * @brief Mathematical Expression Parser
 * 
 * Demonstrates recursive descent parsing with algebraic composition.
 */
class ExpressionParser {
public:
    enum class OpType { Add, Subtract, Multiply, Divide };
    
    struct Expression {
        virtual ~Expression() = default;
        virtual double evaluate() const = 0;
        virtual std::string to_string() const = 0;
    };
    
    struct Number : Expression {
        double value;
        explicit Number(double v) : value(v) {}
        double evaluate() const override { return value; }
        std::string to_string() const override { return std::to_string(value); }
    };
    
    struct BinaryOp : Expression {
        std::unique_ptr<Expression> left;
        OpType op;
        std::unique_ptr<Expression> right;
        
        BinaryOp(std::unique_ptr<Expression> l, OpType o, std::unique_ptr<Expression> r)
            : left(std::move(l)), op(o), right(std::move(r)) {}
        
        double evaluate() const override {
            double l_val = left->evaluate();
            double r_val = right->evaluate();
            
            switch (op) {
                case OpType::Add: return l_val + r_val;
                case OpType::Subtract: return l_val - r_val;
                case OpType::Multiply: return l_val * r_val;
                case OpType::Divide: return l_val / r_val;
            }
            return 0.0;
        }
        
        std::string to_string() const override {
            char op_char = op == OpType::Add ? '+' :
                          op == OpType::Subtract ? '-' :
                          op == OpType::Multiply ? '*' : '/';
            return "(" + left->to_string() + " " + op_char + " " + right->to_string() + ")";
        }
    };
    
    std::optional<std::unique_ptr<Expression>> parse_expression(std::string_view input) const {
        auto tokens = tokenize_math(input);
        if (!tokens || tokens->empty()) {
            return {};
        }
        
        size_t pos = 0;
        return parse_add_subtract(*tokens, pos);
    }
    
private:
    enum class TokenType { Number, Plus, Minus, Multiply, Divide, LeftParen, RightParen };
    
    struct Token {
        TokenType type;
        std::string value;
    };
    
    std::optional<std::vector<Token>> tokenize_math(std::string_view input) const {
        std::vector<Token> tokens;
        
        for (size_t i = 0; i < input.size(); ++i) {
            char c = input[i];
            
            if (std::isspace(c)) continue;
            
            if (std::isdigit(c) || c == '.') {
                std::string number;
                while (i < input.size() && (std::isdigit(input[i]) || input[i] == '.')) {
                    number += input[i++];
                }
                --i; // Back up one
                tokens.push_back({TokenType::Number, number});
            } else {
                switch (c) {
                    case '+': tokens.push_back({TokenType::Plus, "+"}); break;
                    case '-': tokens.push_back({TokenType::Minus, "-"}); break;
                    case '*': tokens.push_back({TokenType::Multiply, "*"}); break;
                    case '/': tokens.push_back({TokenType::Divide, "/"}); break;
                    case '(': tokens.push_back({TokenType::LeftParen, "("}); break;
                    case ')': tokens.push_back({TokenType::RightParen, ")"}); break;
                    default: return {}; // Invalid character
                }
            }
        }
        
        return tokens;
    }
    
    // Recursive descent parser methods
    std::optional<std::unique_ptr<Expression>> parse_add_subtract(std::vector<Token> const& tokens, size_t& pos) const {
        auto left = parse_multiply_divide(tokens, pos);
        if (!left) return {};
        
        while (pos < tokens.size() && 
               (tokens[pos].type == TokenType::Plus || tokens[pos].type == TokenType::Minus)) {
            OpType op = tokens[pos].type == TokenType::Plus ? OpType::Add : OpType::Subtract;
            ++pos;
            
            auto right = parse_multiply_divide(tokens, pos);
            if (!right) return {};
            
            left = std::make_unique<BinaryOp>(std::move(*left), op, std::move(*right));
        }
        
        return left;
    }
    
    std::optional<std::unique_ptr<Expression>> parse_multiply_divide(std::vector<Token> const& tokens, size_t& pos) const {
        auto left = parse_primary(tokens, pos);
        if (!left) return {};
        
        while (pos < tokens.size() && 
               (tokens[pos].type == TokenType::Multiply || tokens[pos].type == TokenType::Divide)) {
            OpType op = tokens[pos].type == TokenType::Multiply ? OpType::Multiply : OpType::Divide;
            ++pos;
            
            auto right = parse_primary(tokens, pos);
            if (!right) return {};
            
            left = std::make_unique<BinaryOp>(std::move(*left), op, std::move(*right));
        }
        
        return left;
    }
    
    std::optional<std::unique_ptr<Expression>> parse_primary(std::vector<Token> const& tokens, size_t& pos) const {
        if (pos >= tokens.size()) return {};
        
        if (tokens[pos].type == TokenType::Number) {
            double value = std::stod(tokens[pos].value);
            ++pos;
            return std::make_unique<Number>(value);
        }
        
        if (tokens[pos].type == TokenType::LeftParen) {
            ++pos; // Skip '('
            auto expr = parse_add_subtract(tokens, pos);
            if (!expr || pos >= tokens.size() || tokens[pos].type != TokenType::RightParen) {
                return {};
            }
            ++pos; // Skip ')'
            return expr;
        }
        
        return {};
    }
};

} // namespace dsl
} // namespace algebraic_parsers
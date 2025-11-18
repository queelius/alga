#pragma once

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <optional>
#include <string>
#include <algorithm>
#include <cstring>

namespace alga {
namespace streaming {

/**
 * @brief Buffered stream reader for efficient parsing
 *
 * Provides a buffered view over an input stream, allowing look-ahead
 * without loading the entire file into memory.
 */
class BufferedStreamReader {
private:
    std::istream& stream;
    std::vector<char> buffer;
    size_t buffer_size;
    size_t current_pos;
    size_t valid_size;  // How much of buffer contains valid data
    bool eof_reached;

    void fill_buffer() {
        // Move any remaining data to the beginning
        if (current_pos < valid_size) {
            size_t remaining = valid_size - current_pos;
            memmove(buffer.data(), buffer.data() + current_pos, remaining);
            valid_size = remaining;
        } else {
            valid_size = 0;
        }
        current_pos = 0;

        // Fill the rest of the buffer
        stream.read(buffer.data() + valid_size, buffer_size - valid_size);
        valid_size += stream.gcount();

        if (stream.eof() || stream.fail()) {
            eof_reached = true;
        }
    }

public:
    explicit BufferedStreamReader(std::istream& is, size_t buf_size = 4096)
        : stream(is), buffer(buf_size), buffer_size(buf_size),
          current_pos(0), valid_size(0), eof_reached(false)
    {
        fill_buffer();
    }

    /**
     * @brief Peek at the current character without consuming it
     */
    std::optional<char> peek() {
        if (current_pos >= valid_size) {
            if (eof_reached) {
                return std::nullopt;
            }
            fill_buffer();
            if (current_pos >= valid_size) {
                return std::nullopt;
            }
        }
        return buffer[current_pos];
    }

    /**
     * @brief Get current character and advance
     */
    std::optional<char> get() {
        auto ch = peek();
        if (ch) {
            ++current_pos;
        }
        return ch;
    }

    /**
     * @brief Peek ahead N characters
     */
    std::optional<char> peek_ahead(size_t n) {
        // Ensure we have enough data buffered
        size_t needed_pos = current_pos + n;

        while (needed_pos >= valid_size && !eof_reached) {
            fill_buffer();
        }

        if (needed_pos >= valid_size) {
            return std::nullopt;
        }

        return buffer[needed_pos];
    }

    /**
     * @brief Read up to N characters into a string
     */
    std::string read_string(size_t n) {
        std::string result;
        result.reserve(n);

        for (size_t i = 0; i < n; ++i) {
            auto ch = get();
            if (!ch) break;
            result += *ch;
        }

        return result;
    }

    /**
     * @brief Read until a predicate returns false
     */
    template<typename Predicate>
    std::string read_while(Predicate pred) {
        std::string result;

        while (true) {
            auto ch = peek();
            if (!ch || !pred(*ch)) {
                break;
            }
            result += *ch;
            get();  // Consume it
        }

        return result;
    }

    /**
     * @brief Check if at end of stream
     */
    bool at_end() {
        return current_pos >= valid_size && eof_reached;
    }

    /**
     * @brief Get current position in stream
     */
    size_t position() const {
        auto pos = stream.tellg();
        if (pos == -1) return 0;
        return static_cast<size_t>(pos) - (valid_size - current_pos);
    }
};

/**
 * @brief Line-by-line stream parser
 *
 * Processes input stream one line at a time, useful for line-oriented parsing.
 */
template<typename Parser>
class LineParser {
private:
    Parser parser;

public:
    explicit LineParser(Parser p) : parser(std::move(p)) {}

    /**
     * @brief Parse stream line by line
     *
     * Returns vector of results, one per line.
     * Lines that fail to parse are skipped (optional in result vector is nullopt).
     */
    auto parse_stream(std::istream& stream) const
        -> std::vector<std::optional<typename Parser::output_type>>
    {
        using output_t = typename Parser::output_type;
        std::vector<std::optional<output_t>> results;
        std::string line;

        while (std::getline(stream, line)) {
            auto [pos, result] = parser.parse(line.begin(), line.end());
            results.push_back(std::move(result));
        }

        return results;
    }

    /**
     * @brief Parse stream with callback per line
     *
     * Calls callback for each parsed line. More memory-efficient than parse_stream.
     */
    template<typename Callback>
    void parse_with_callback(std::istream& stream, Callback callback) const {
        std::string line;
        size_t line_number = 0;

        while (std::getline(stream, line)) {
            ++line_number;
            auto [pos, result] = parser.parse(line.begin(), line.end());
            callback(line_number, line, std::move(result));
        }
    }
};

/**
 * @brief Create a line parser
 */
template<typename Parser>
LineParser<Parser> by_line(Parser parser) {
    return LineParser<Parser>(std::move(parser));
}

/**
 * @brief Chunked stream parser
 *
 * Processes input stream in fixed-size chunks.
 */
template<typename Parser>
class ChunkParser {
private:
    Parser parser;
    size_t chunk_size;

public:
    ChunkParser(Parser p, size_t chunk_sz)
        : parser(std::move(p)), chunk_size(chunk_sz) {}

    /**
     * @brief Parse stream in chunks
     *
     * Returns vector of results, one per chunk that successfully parsed.
     */
    auto parse_stream(std::istream& stream) const
        -> std::vector<std::optional<typename Parser::output_type>>
    {
        using output_t = typename Parser::output_type;
        std::vector<std::optional<output_t>> results;
        std::vector<char> buffer(chunk_size);

        while (stream.read(buffer.data(), chunk_size) || stream.gcount() > 0) {
            size_t bytes_read = stream.gcount();
            auto [pos, result] = parser.parse(buffer.begin(), buffer.begin() + bytes_read);
            results.push_back(std::move(result));
        }

        return results;
    }

    /**
     * @brief Parse stream with callback per chunk
     */
    template<typename Callback>
    void parse_with_callback(std::istream& stream, Callback callback) const {
        std::vector<char> buffer(chunk_size);
        size_t chunk_number = 0;

        while (stream.read(buffer.data(), chunk_size) || stream.gcount() > 0) {
            ++chunk_number;
            size_t bytes_read = stream.gcount();
            auto [pos, result] = parser.parse(buffer.begin(), buffer.begin() + bytes_read);
            callback(chunk_number, bytes_read, std::move(result));
        }
    }
};

/**
 * @brief Create a chunk parser
 */
template<typename Parser>
ChunkParser<Parser> by_chunks(Parser parser, size_t chunk_size = 4096) {
    return ChunkParser<Parser>(std::move(parser), chunk_size);
}

/**
 * @brief Streaming file parser - convenience wrapper
 *
 * Opens a file and applies parser to it.
 */
template<typename Parser>
class FileParser {
private:
    Parser parser;
    std::string filepath;

public:
    FileParser(std::string path, Parser p)
        : parser(std::move(p)), filepath(std::move(path)) {}

    /**
     * @brief Parse entire file into memory, then apply parser
     *
     * Use this for small to medium files.
     */
    auto parse() const -> std::optional<typename Parser::output_type> {
        std::ifstream file(filepath);
        if (!file) {
            return std::nullopt;
        }

        // Read entire file
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

        auto [pos, result] = parser.parse(content.begin(), content.end());
        return result;
    }

    /**
     * @brief Parse file line by line
     *
     * More memory-efficient for large files.
     */
    template<typename Callback>
    bool parse_by_line(Callback callback) const {
        std::ifstream file(filepath);
        if (!file) {
            return false;
        }

        auto line_parser = by_line(parser);
        line_parser.parse_with_callback(file, callback);
        return true;
    }

    /**
     * @brief Parse file in chunks
     */
    template<typename Callback>
    bool parse_by_chunks(Callback callback, size_t chunk_size = 4096) const {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            return false;
        }

        auto chunk_parser = by_chunks(parser, chunk_size);
        chunk_parser.parse_with_callback(file, callback);
        return true;
    }
};

/**
 * @brief Create a file parser
 */
template<typename Parser>
FileParser<Parser> from_file(std::string filepath, Parser parser) {
    return FileParser<Parser>(std::move(filepath), std::move(parser));
}

/**
 * @brief Stream combinator - applies parser repeatedly to stream
 *
 * Keeps parsing until stream is exhausted or parser fails.
 */
template<typename Parser>
class StreamCombinator {
private:
    Parser parser;

public:
    explicit StreamCombinator(Parser p) : parser(std::move(p)) {}

    /**
     * @brief Parse stream repeatedly until exhausted
     *
     * Returns all successful parses. Stops on first failure.
     */
    auto parse_all(std::istream& stream) const
        -> std::vector<typename Parser::output_type>
    {
        using output_t = typename Parser::output_type;
        std::vector<output_t> results;

        BufferedStreamReader reader(stream);

        while (!reader.at_end()) {
            // Read what we can into a string for the parser
            std::string chunk = reader.read_while([](char) { return true; });
            if (chunk.empty()) break;

            auto [pos, result] = parser.parse(chunk.begin(), chunk.end());
            if (!result) {
                break;  // Stop on first parse failure
            }

            results.push_back(std::move(*result));

            // If we didn't consume all input, we're stuck
            if (pos != chunk.end()) {
                break;
            }
        }

        return results;
    }
};

/**
 * @brief Create a stream combinator
 */
template<typename Parser>
StreamCombinator<Parser> stream_many(Parser parser) {
    return StreamCombinator<Parser>(std::move(parser));
}

} // namespace streaming
} // namespace alga

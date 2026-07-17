#pragma once

#include <vector>
#include <string>
#include <unordered_map>

enum class ParseState{
    READING_HEADERS,
    READING_BODY,
    COMPLETE,
    ERROR_STATE
};

class HttpRequest{
public:
    HttpRequest() = default;

    void feed(const unsigned char* data, size_t length);
    ParseState state() const;

    std::string headers(const std::string& key);
    std::string pipeline() const;
    std::string file_format() const;
    std::string file_type() const;
    const std::vector<unsigned char>& file_bytes() const;
    void reset();
private:
    std::vector<unsigned char> buffer_;
    ParseState state_ = ParseState::READING_HEADERS;
    std::vector<unsigned char> file_bytes_;
    std::string pipeline_;
    std::string file_type_;
    std::string file_format_;
    std::unordered_map<std::string, std::string> headers_;
    void parse_headers(std::string_view header_block);
    void parse_body(std::vector<unsigned char>::const_iterator body_start, std::vector<unsigned char>::const_iterator body_end);
    size_t body_start_offset_ = 0;
};
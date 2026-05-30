#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "tcp_connection.hpp"

class HttpRequest{
public:
    HttpRequest(TCPConnection conn);
    const std::vector<unsigned char>& file_bytes() const;
    std::string headers(const std::string& key);
    std::string pipeline() const;
    std::string file_format() const;
    std::string file_type() const;
private:
    std::vector<unsigned char> file_bytes_;
    std::string pipeline_;
    std::string file_type_;
    std::string file_format_;
    std::unordered_map<std::string, std::string> headers_;
    void parse_headers(const std::string& header_block);
    void parse_body(const std::vector<unsigned char>& body_block);
};
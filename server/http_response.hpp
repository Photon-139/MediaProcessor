#pragma once
#include <vector>
#include <string>

class HttpResponse{
public:
    HttpResponse()=default;
    void load(std::string&& headers, std::vector<unsigned char>&& body);
    bool write_some(const int client_fd);
    void reset();
    bool has_pending_data();
private:
    size_t bytes_sent_ = 0;
    std::vector<unsigned char> file_bytes_;
    std::string headers_;
};
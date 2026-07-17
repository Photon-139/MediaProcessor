#pragma once
#include "http_request.hpp"
#include "http_response.hpp"
#include <cstdint>

class TCPConnection{
    public:
        explicit TCPConnection(int fd, uint64_t id);
        ~TCPConnection();
        int fd() const;
        TCPConnection(const TCPConnection&) = delete;
        TCPConnection& operator=(const TCPConnection&) = delete;
        TCPConnection(TCPConnection&& other) noexcept;
        TCPConnection& operator=(TCPConnection&& other) noexcept;
        HttpRequest& request();
        HttpResponse& response();   
        uint64_t id() const;
    private:
        int fd_;
        uint64_t id_;
        HttpRequest request_;
        HttpResponse response_;
};
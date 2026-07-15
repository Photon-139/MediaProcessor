#pragma once
#include "http_request.hpp"

class TCPConnection{
    public:
        explicit TCPConnection(int fd);
        ~TCPConnection();
        int fd() const;
        TCPConnection(const TCPConnection&) = delete;
        TCPConnection& operator=(const TCPConnection&) = delete;
        TCPConnection(TCPConnection&& other) noexcept;
        TCPConnection& operator=(TCPConnection&& other) noexcept;
        HttpRequest& request();
    private:
        int fd_;
        HttpRequest request_;
};
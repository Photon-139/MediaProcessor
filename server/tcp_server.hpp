#pragma once
#include "tcp_connection.hpp"

class TCPServer{
    public:
        TCPServer(int port);
        ~TCPServer();
        TCPConnection accept_conn();
        int fd() const;
    private:
        int fd_;
};
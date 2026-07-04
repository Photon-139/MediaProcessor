#include "tcp_connection.hpp"
#include <unistd.h>
#include <algorithm>

TCPConnection::TCPConnection(int fd){
    fd_ = fd;
}

int TCPConnection::fd() const{
    return fd_;
}
TCPConnection::~TCPConnection(){
    close(fd_);
}

TCPConnection::TCPConnection(TCPConnection&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

TCPConnection& TCPConnection::operator=(TCPConnection&& other) noexcept{
    if(this!=&other){
        std::swap(fd_, other.fd_);
    }
    return *this;
}
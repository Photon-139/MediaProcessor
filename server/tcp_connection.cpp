#include "tcp_connection.hpp"
#include <unistd.h>
#include <algorithm>
#include <cstdint>

TCPConnection::TCPConnection(int fd, uint64_t id) : fd_(fd), id_(id){
}

int TCPConnection::fd() const{
    return fd_;
}
TCPConnection::~TCPConnection(){
    close(fd_);
}

TCPConnection::TCPConnection(TCPConnection&& other) noexcept : fd_(other.fd_), request_(std::move(other.request_)) {
    other.fd_ = -1;
}

TCPConnection& TCPConnection::operator=(TCPConnection&& other) noexcept{
    if(this!=&other){
        std::swap(fd_, other.fd_);
        std::swap(request_, other.request_);
    }
    return *this;
}

HttpRequest& TCPConnection::request(){
    return request_;
}

HttpResponse& TCPConnection::response(){
    return response_;
}

uint64_t TCPConnection::id() const{
    return id_;
}
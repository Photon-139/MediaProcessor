#include "tcp_connection.hpp"
#include <unistd.h>

TCPConnection::TCPConnection(int fd){
    fd_ = fd;
}

int TCPConnection::fd() const{
    return fd_;
}
TCPConnection::~TCPConnection(){
    close(fd_);
}
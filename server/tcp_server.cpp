#include "tcp_server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <spdlog/spdlog.h>

bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return (flags != -1) && (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0);
}


TCPServer::TCPServer(int port){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd<0){
        throw std::runtime_error("Could not create server socket");
    }
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    int opt = 1;
    fd_ = server_fd;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("setsockopt error");

    }
    if(bind(server_fd, (sockaddr*)&address, sizeof(address))<0){
        throw std::runtime_error("Could not bind");
    }

    if(listen(server_fd, 10)<0){
        throw std::runtime_error("Failed to listen on socket: " + std::to_string(server_fd));
    }
    
    if(!setNonBlocking(fd_)){
        throw std::runtime_error("Could not set the server fd to non-blocking");
    }
}

TCPConnection TCPServer::accept_conn(){
    int client_socket = accept(fd_, nullptr, nullptr);
    if(client_socket<0){
        if(errno==EAGAIN || errno==EWOULDBLOCK){
            return TCPConnection(-1, 0);
        }
        throw std::runtime_error("Accept failed");
    }
    if(!setNonBlocking(client_socket)){
        throw std::runtime_error("Could not set the following socket to non-block: "+std::to_string(client_socket));
    }
    return TCPConnection(client_socket, next_conn_id_.fetch_add(1));
}

TCPServer::~TCPServer(){
    close(fd_);
}

int TCPServer::fd() const{
    return fd_;
}
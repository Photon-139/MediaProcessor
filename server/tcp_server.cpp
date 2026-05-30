#include "tcp_server.hpp"
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>

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
        throw std::runtime_error("Failed to list on socket: " + std::to_string(server_fd));
    }
}

TCPConnection TCPServer::accept_conn(){
    int client_socket = accept(fd_, nullptr, nullptr);
    return TCPConnection(client_socket);
}

TCPServer::~TCPServer(){
    close(fd_);
}
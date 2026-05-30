#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include "./server/http_request.hpp"

#define PORT 8080

int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd<0){
        std::cout << "Error in creating socket" << std::endl;
        exit(1);
    }
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }
    if(bind(server_fd, (sockaddr*)&address, sizeof(address))<0){
        std::cout << "Bind failed" << std::endl;
        exit(1);
    }

    if(listen(server_fd, 10)<0){
        std::cout << "Listen failed" << std::endl;
        exit(1);
    }
    while(1){
        std::cout << "Server waiting for connection on " << PORT << std::endl;
        int client_socket = accept(server_fd, nullptr, nullptr);
        if(client_socket<0){
            std::cout << "Accept failed" << std::endl;
            exit(1);
        }
        HttpRequest req = HttpRequest(client_socket);
        

    }

}
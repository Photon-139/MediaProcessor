#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include "./server/http_request.hpp"
#include "./server/tcp_server.hpp"
#include "./server/tcp_connection.hpp"
#define PORT 8080

int main(){

    TCPServer tcpServer = TCPServer(PORT);

    while(1){
        std::cout << "Server waiting for connection on port: " << PORT <<std::endl;
        TCPConnection tcpConn = tcpServer.accept_conn();
        HttpRequest req = HttpRequest(tcpConn);
        std::cout << req.file_format() << std::endl;
        std::cout << req.file_type() << std::endl;
        std::cout << req.pipeline() << std::endl;
    }

}
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include "./server/http_request.hpp"
#include "./server/tcp_server.hpp"
#include "./server/tcp_connection.hpp"
#include "./transform/pipeline.hpp"
#define PORT 8080

int main(){

    TCPServer tcpServer = TCPServer(PORT);

    while(1){
        std::cout << "Server waiting for connection on port: " << PORT <<std::endl;
        TCPConnection tcpConn = tcpServer.accept_conn();
        while(1){
            HttpRequest req = HttpRequest(tcpConn);
            if(!req.isValid()) break;
            std::string header_method = req.headers("method");
            if(header_method=="OPTIONS"){
                std::string preflight = 
                    "HTTP/1.1 204 No Content\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n";
                send(tcpConn.fd(), preflight.data(), preflight.size(), 0);
            }else if(header_method=="POST"){
                Pipeline pipeline = Pipeline(req.pipeline(), req.file_type(), req.file_format());
                std::vector<unsigned char> res = pipeline.process(req.file_bytes());
                std::string res_headers =
                            "HTTP/1.1 200 OK\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Connection: keep-alive\r\n"    
                            "Content-Type: " + req.file_type() + "/" + req.file_format() + "\r\n"
                            "Content-Length: " + std::to_string(res.size()) + "\r\n"
                            "\r\n";
                int header_response = send(tcpConn.fd(), res_headers.data(), res_headers.size(), 0);
                std::cout << "Header resposne code: " << header_response << std::endl;
                if(header_response<0){
                    std::cout << "Send failed: " << strerror(errno) << std::endl;
                    break;
                }
                int file_response = send(tcpConn.fd(), res.data(), res.size(), 0);
                if(file_response<0){
                    std::cout << "Send failed: " << strerror(errno) << std::endl;
                    break;
                }
                std::cout << "File response code " << file_response << std::endl;
            }
            

        }
    }

}
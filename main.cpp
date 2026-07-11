#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <utility>
#include "./concurrency/thread_pool.hpp"
#include "./server/http_request.hpp"
#include "./server/tcp_connection.hpp"
#include "./server/tcp_server.hpp"
#include "./transform/pipeline.hpp"
#include <atomic>
#include <csignal>
#define PORT 8080
#define NETWORK_THREADS 4

std::atomic<bool> running{true};

void handle_shutdown(int signal){
    spdlog::warn("Shutdown signal received");
    running.store(false);
}

void handle_connection(TCPConnection tcpConn){
    try{
        while (1) {
            HttpRequest req = HttpRequest(tcpConn);
            if (!req.isValid()) break;
            std::string header_method = req.headers("method");
            if (header_method == "OPTIONS") {
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
                    "Content-Type: " + req.file_type() + "/" +(req.file_type() == "image" ? req.file_format() : "wav")+"\r\n"
                    "Content-Length: " + std::to_string(res.size()) +"\r\n"
                    "\r\n";
                int header_response = send(tcpConn.fd(), res_headers.data(), res_headers.size(), 0);
                spdlog::info("Header resposne code: {}", header_response);
                if(header_response < 0){
                    spdlog::error("Send failed: {}", strerror(errno));
                    break;
                }
                size_t bytes_sent = 0;
                ssize_t file_response;
                while(bytes_sent < res.size()){
                    file_response = send(tcpConn.fd(), res.data() + bytes_sent, static_cast<size_t>(res.size() - bytes_sent), 0);
                    if (file_response < 0) break;
                    bytes_sent += static_cast<size_t>(file_response);
                }
                if(file_response < 0){
                    spdlog::error("Send failed: {}", strerror(errno));
                    break;
                }
                spdlog::info("File response code: {}", file_response);
            }
        }
    }catch(const std::exception& e){
        spdlog::error("Connection error: {}", e.what());
    }catch(...){
        spdlog::error("Connection error: Unknown exception");
    }
}

int main() {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
    std::signal(SIGTERM, handle_shutdown);
    std::signal(SIGINT, handle_shutdown);
    TCPServer tcpServer = TCPServer(PORT);
    
    ThreadPool pool(NETWORK_THREADS);
    spdlog::info("Server waiting for connection on port {}", PORT);
    while (running) {
        try{
            TCPConnection tcpConn = tcpServer.accept_conn();
            if(tcpConn.fd()<0) continue;
            spdlog::info("Accepted new connection with fd: {}", tcpConn.fd());
            pool.submit([conn = std::move(tcpConn)]() mutable {
                handle_connection(std::move(conn));
            });
        }catch(const std::exception& e){
            spdlog::error("Accept error: {}", e.what());
        }
    }
}
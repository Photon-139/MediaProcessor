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
#include <fcntl.h>
#include <sys/epoll.h>

constexpr int PORT = 8080;
constexpr int MAX_EVENTS = 10;
constexpr int BUFFER_SIZE = 4096;

bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return (flags != -1) && (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0);
}

std::atomic<bool> running{true};

void handle_shutdown(int signal){
    spdlog::warn("Shutdown signal received");
    running.store(false);
}
int main() {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
    std::signal(SIGTERM, handle_shutdown);
    std::signal(SIGINT, handle_shutdown);
    TCPServer tcpServer = TCPServer(PORT);
    int serverFd = tcpServer.fd();
    int epollFd = epoll_create1(0);
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = serverFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev);
    std::unordered_map<int, TCPConnection> clientMap;

    spdlog::info("Server waiting for connection on port {}", PORT);

    std::vector<epoll_event> events(MAX_EVENTS);

    unsigned char buffer[BUFFER_SIZE];

    while(running){
        int numEvents = epoll_wait(epollFd, events.data(), MAX_EVENTS, -1);
        for(int i = 0; i<numEvents; ++i){
            if(events[i].data.fd==serverFd){
                while(true){
                    TCPConnection tcpConn = tcpServer.accept_conn();
                    if(tcpConn.fd()==-1) break;
                    int clientFd = tcpConn.fd();

                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = clientFd;
                    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev)==-1){
                        throw std::runtime_error("Faied to add the following socket to epoll: " + std::to_string(clientFd));
                    }
                    clientMap[clientFd] = std::move(tcpConn);
                }
            }else{
                while(1){
                    ssize_t n = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
                    if(n==-1){
                        if(errno==EAGAIN || errno==EWOULDBLOCK) break;
                        // TODO: Handle actual socket error
                    }
                    if(n==0){
                        clientMap.erase(events[i].data.fd);
                        break;
                    }
                    int clientFd = events[i].data.fd;
                    auto& conn = clientMap[clientFd];
                    auto& req = conn.request();
                    req.feed(buffer, n);
                    if(req.state()==ParseState::ERROR_STATE){
                        // Do something
                        break;
                    }
                    if(req.state()!=ParseState::COMPLETE) continue;
                    std::string method = req.headers("method");
                    if(method=="OPTIONS"){
                        std::string preflight =
                            "HTTP/1.1 204 No Content\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
                            "Access-Control-Allow-Headers: Content-Type\r\n"
                            "Connection: keep-alive\r\n"
                            "\r\n";
                            send(clientFd, preflight.data(), preflight.size(), 0);
                    }else if(method=="POST"){
                        Pipeline pipeline(req.pipeline(), req.file_type(), req.file_format());
                        std::vector<unsigned char> res = pipeline.process(req.file_bytes());
                        std::string res_headers =
                            "HTTP/1.1 200 OK\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Connection: keep-alive\r\n"
                            "Content-Type: " + req.file_type() + "/" +(req.file_type() == "image" ? req.file_format() : "wav")+"\r\n"
                            "Content-Length: " + std::to_string(res.size()) +"\r\n"
                            "\r\n";
                        int header_response = send(clientFd, res_headers.data(), res_headers.size(), 0);
                        spdlog::info("Header resposne code: {}", header_response);
                        if(header_response < 0){
                            spdlog::error("Send failed: {}", strerror(errno));
                            break;
                        }
                        size_t bytes_sent = 0;
                        ssize_t file_response;
                        while(bytes_sent < res.size()){
                            file_response = send(clientFd, res.data() + bytes_sent, static_cast<size_t>(res.size() - bytes_sent), 0);
                            if (file_response < 0) break;
                            bytes_sent += static_cast<size_t>(file_response);
                        }
                        if(file_response < 0){
                            spdlog::error("Send failed: {}", strerror(errno));
                            break;
                        }
                        spdlog::info("File response code: {}", file_response);
                        req.reset();
                    }
                }
            }
        }
    }
}
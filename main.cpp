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
#include <sys/eventfd.h>
#include "./concurrency/safe_queue.hpp"

constexpr int PORT = 8080;
constexpr int MAX_EVENTS = 10;
constexpr int BUFFER_SIZE = 4096;
constexpr int COMPUTE_THREADS = 10;

struct ComputeResult{
    int client_fd;
    uint64_t conn_id;
    std::string headers;
    std::vector<unsigned char> body;
};

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
    if(epollFd<0){
        spdlog::error("Failed to create epoll instance");
        return 1;
    }

    int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(efd<0){
        close(epollFd);
        spdlog::error("Failed to create eventfd");
        return 1;
    }
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = serverFd;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev)==-1){
        close(efd);
        close(epollFd);
        spdlog::error("Failed to add serverfd to epoll");
        return 1;
    }
    ev.data.fd = efd;
    ev.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, efd, &ev)==-1){
        close(efd);
        close(epollFd);
        spdlog::error("Failed to add eventfd to epoll");
        return 1;
    }
    ThreadPool thread_pool(COMPUTE_THREADS);
    SafeQueue<ComputeResult> result_queue;
    

    std::unordered_map<int, TCPConnection> clientMap;

    spdlog::info("Server waiting for connection on port {}", PORT);

    std::vector<epoll_event> events(MAX_EVENTS);

    unsigned char buffer[BUFFER_SIZE];

    while(running){
        int numEvents = epoll_wait(epollFd, events.data(), MAX_EVENTS, -1);

        if(numEvents==-1){
            if(errno!=EINTR){
                spdlog::error("epoll_wait failed: {}", strerror(errno));
            }
            // Other signals also fall here, consider how we handle them
            continue;
        }
        for(int i = 0; i<numEvents; ++i){
            if(events[i].data.fd==serverFd){
                while(true){
                    TCPConnection tcpConn = tcpServer.accept_conn();
                    if(tcpConn.fd()==-1) break;
                    int clientFd = tcpConn.fd();

                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = clientFd;
                    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev)==-1){
                        spdlog::error("Failed to add the following socket to epoll: {}", std::to_string(clientFd));
                        continue;
                    }
                    clientMap.emplace(clientFd, std::move(tcpConn));
                    spdlog::info("Client accepts with fd: {}", clientFd);
                }
            }else if(events[i].data.fd==efd){
                spdlog::info("Event received for efd");
                uint64_t count;
                while(true){
                    if(read(efd, &count, sizeof(count)) > 0) continue;
                    if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                    spdlog::error("eventfd read failed: {}", strerror(errno));
                    break;
                }
                ComputeResult cr;
                while(result_queue.try_pop(cr)){
                    auto it = clientMap.find(cr.client_fd);
                    if(it==clientMap.end() || it->second.id()!=cr.conn_id){
                        spdlog::warn("Stale job for fd {} (Client disconnected or fd recycled). Dropping.", cr.client_fd);
                        continue;
                    }
                    auto& conn = it->second;
                    conn.response().load(std::move(cr.headers), std::move(cr.body));
                    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                    ev.data.fd = cr.client_fd;
                    if(epoll_ctl(epollFd, EPOLL_CTL_MOD, cr.client_fd, &ev)==-1){
                        spdlog::error("Failed to add to EPOLLOUT to fd {}: {}", cr.client_fd, strerror(errno));
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, cr.client_fd, nullptr);
                        clientMap.erase(it);
                        continue;
                    }
                    spdlog::info("fd: {} added to EPOLLOUT", cr.client_fd);
                }
            }else{
                bool disconnected = false;
                int fd_ = events[i].data.fd;
                if(events[i].events & EPOLLIN){
                    spdlog::info("Receiving data on fd: {}", fd_);
                    while(1){
                        ssize_t n = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
                        if(n==-1){
                            if(errno==EAGAIN || errno==EWOULDBLOCK) break;
                            spdlog::error("recv failed on {}: {}", fd_, strerror(errno));
                            if(epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr)==-1){
                                spdlog::error("Failed to remove fd: {} from epoll: {}", fd_, strerror(errno));
                            }
                            clientMap.erase(events[i].data.fd);
                            disconnected = true;
                            break;
                        }
                        if(n==0){
                            if(epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr)==-1){
                                spdlog::error("Failed to remove fd: {} from epoll: {}", fd_, strerror(errno));
                            }
                            clientMap.erase(events[i].data.fd);
                            disconnected = true;
                            break;
                        }
                        int clientFd = events[i].data.fd;
                        auto it = clientMap.find(clientFd);
                        if(it==clientMap.end()){
                            spdlog::warn("Client with fd: {} register for EPOLLIN but absent from clientMap", fd_);
                            // remove from epoll maybe
                            if(epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr)==-1){
                                spdlog::error("Failed to remove fd: {} from epoll: {}", fd_, strerror(errno));
                            }
                            disconnected = true;
                            break;
                        }
                        auto& conn = it->second;
                        auto& req = conn.request();
                        req.feed(buffer, n);
                        if(req.state()==ParseState::ERROR_STATE){
                            spdlog::error("Failed to parse request on fd: {}", fd_);
                            // Do something

                            // Think about whether to close the connection or keep it alive 
                            std::string body = "Bad Request";
                            std::string headers =
                                "HTTP/1.1 400 Bad Request\r\n"
                                "Access-Control-Allow-Origin: *\r\n"
                                "Connection: keep-alive\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: " + std::to_string(body.size()) + "\r\n"
                                "\r\n";
                            conn.response().load(std::move(headers), std::vector<unsigned char>(body.begin(), body.end()));
                            ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                            ev.data.fd = clientFd;
                            if(epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev)==-1){
                                spdlog::error("Failed to mod epoll for fd {}: {}", clientFd, strerror(errno));
                                epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
                                clientMap.erase(clientFd);
                                disconnected = true;
                                break;
                            }
                            req.reset();
                            break;
                        }
                        if(req.state()!=ParseState::COMPLETE) continue;

                        auto method_opt = req.headers("method");
                        

                        std::string method = *method_opt;
                        // Technically shouldn't dereference but if we have reached this point without ERROR_STATE, headers will have method
                        // Consider value_or but fine for now, no further discussion 

                        if(method=="OPTIONS"){
                            std::string preflight =
                                "HTTP/1.1 204 No Content\r\n"
                                "Access-Control-Allow-Origin: *\r\n"
                                "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
                                "Access-Control-Allow-Headers: Content-Type\r\n"
                                "Connection: keep-alive\r\n"
                                "\r\n";
                            conn.response().load(std::move(preflight), std::vector<unsigned char>{});
                            ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                            ev.data.fd = clientFd;
                            if(epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev)==-1){
                                spdlog::error("Failed to mod epoll for fd {}: {}", clientFd, strerror(errno));
                                epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
                                clientMap.erase(clientFd);
                                disconnected = true;
                                break;
                            }
                            req.reset();
                        }else if(method=="POST"){
                            uint64_t connId = conn.id();
                            std::string pipeline_str = req.pipeline();
                            std::string file_type = req.file_type();
                            std::string file_format = req.file_format();
                            std::vector<unsigned char> file_bytes = req.take_file_bytes();
                            thread_pool.submit([
                                clientFd,
                                connId,
                                efd,
                                &result_queue,
                                pipeline_str = std::move(pipeline_str),
                                file_type = std::move(file_type),
                                file_format = std::move(file_format),
                                file_bytes = std::move(file_bytes)
                            ]() mutable {
                                Pipeline pipeline(pipeline_str, file_type, file_format);
                                spdlog::info("Pipeline with parameters constructed: {} {} {}", pipeline_str, file_type, file_format);
                                std::vector<unsigned char> res_file_bytes = pipeline.process(file_bytes);
                                spdlog::info("Request from client with fd {} processed, res_file_bytes: {}", clientFd, res_file_bytes.size());
                                    std::string res_headers =
                                        "HTTP/1.1 200 OK\r\n"
                                        "Access-Control-Allow-Origin: *\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Type: " + file_type + "/" +(file_type == "image" ? file_format : "wav")+"\r\n"
                                        "Content-Length: " + std::to_string(res_file_bytes.size()) +"\r\n"
                                        "\r\n";
                                    
                                result_queue.push(ComputeResult{clientFd, connId, std::move(res_headers), std::move(res_file_bytes)});
                                uint64_t signal_val = 1;
                                if(write(efd, &signal_val, sizeof(signal_val)) < 0 && errno != EAGAIN){
                                    spdlog::error("eventfd write failed: {}", strerror(errno));
                                }
                            });
                            req.reset();
                        }
                    }
                } // Should this be an else-if or if
                if(!disconnected && events[i].events & EPOLLOUT){
                    auto it = clientMap.find(events[i].data.fd);
                    if(it==clientMap.end()){
                        int fd_ = events[i].data.fd;
                        spdlog::warn("Client with fd: {} register for EPOLLOUT but absent from clientMap", fd_);
                        if(epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr)==-1){
                            spdlog::error("Failed to remove fd: {} from epoll: {}", fd_, strerror(errno));
                        }
                        continue;
                    }
                    auto& conn = it->second;
                    bool done = false;
                    try{
                        done = conn.response().write_some(conn.fd());
                    }catch(const std::exception& e){
                        spdlog::error("Write error on fd {}: {}", conn.fd(), e.what());
                        if(epoll_ctl(epollFd, EPOLL_CTL_DEL, conn.fd(), nullptr)==-1){
                            spdlog::error("Failed to remove fd: {} from epoll: {}", conn.fd(), strerror(errno));
                        }
                        clientMap.erase(it);
                        continue;
                    }
                    if(done){
                        spdlog::info("Processed response sent back to the client at fd: {}", fd_);
                        conn.response().reset();
                        epoll_event mod_ev{};
                        mod_ev.events = EPOLLIN | EPOLLET;
                        mod_ev.data.fd = conn.fd();
                        if(epoll_ctl(epollFd, EPOLL_CTL_MOD, conn.fd(), &mod_ev)==-1){
                            spdlog::error("Failed to mod epoll for fd {}: {}", conn.fd(), strerror(errno));
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, conn.fd(), nullptr);
                            clientMap.erase(it);
                            continue;
                        }
                    }
                }
            }
        }
    }
}
#include "http_response.hpp"
#include <sys/socket.h>
#include <stdexcept>

void HttpResponse::load(std::string&& headers, std::vector<unsigned char>&& body){
    headers_ = std::move(headers);
    file_bytes_ = std::move(body);
}

void HttpResponse::reset(){
    headers_.clear();
    file_bytes_.clear();
    bytes_sent_ = 0;
}

bool HttpResponse::write_some(int client_fd){
    size_t total_size = headers_.size() + file_bytes_.size();
    while(bytes_sent_<total_size){
        if(bytes_sent_<headers_.size()){
            size_t remaining = headers_.size()-bytes_sent_;
            ssize_t n = send(client_fd, headers_.data()+bytes_sent_, remaining, 0);
            if(n<0){
                if(errno==EAGAIN || errno==EWOULDBLOCK) return false;
                throw std::runtime_error("Header send error on fd: "+std::to_string(client_fd));
            }
            bytes_sent_+=n;
            if(bytes_sent_<headers_.size()) continue;
        }
        size_t offset_in_body = bytes_sent_ - headers_.size();
        size_t remaining = file_bytes_.size() - offset_in_body;
        ssize_t n = send(client_fd, file_bytes_.data()+offset_in_body, remaining, 0);
        if(n<0){
            if(errno==EAGAIN || errno==EWOULDBLOCK) return false;
            throw std::runtime_error("Body send error on fd: "+std::to_string(client_fd));
        }
        bytes_sent_+=n;
    }
    return true;
}

bool HttpResponse::has_pending_data(){
    return !headers_.empty() && (bytes_sent_<(headers_.size() + file_bytes_.size()));
}
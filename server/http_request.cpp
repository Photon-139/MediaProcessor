#include "http_request.hpp"
#include "tcp_connection.hpp"
#include <sys/socket.h>
#include <cstring>
#include <algorithm>
#include <spdlog/spdlog.h>

enum State {INCOMPLETE, COMPLETE, INVALID};

HttpRequest::HttpRequest(const TCPConnection& conn){
    int fd = conn.fd();
    unsigned char buffer[4096];
    std::vector<unsigned char> raw;
    while(1){
        int bytes_received = recv(fd, buffer, sizeof(buffer), 0);
        if(bytes_received<=0){
            spdlog::info("Connection closed or recv failed");
            isValid_ = false;
            break;
        }
        raw.insert(raw.end(), buffer, buffer+bytes_received);
        if(std::search(raw.begin(), raw.end(), "\r\n\r\n", "\r\n\r\n"+4)!=raw.end()) break;
    }
    if(!isValid_) return;
    auto header_boundary = std::search(raw.begin(), raw.end(), "\r\n\r\n", "\r\n\r\n"+4);
    std::string header_block(raw.begin(), header_boundary);
    parse_headers(header_block);

    if(headers_["method"]=="OPTIONS") return;

    int cl = std::stoi(headers_["length"]);

    std::vector<unsigned char> body(header_boundary+4, raw.end());
    while((int)body.size()<cl){
        int bytes_received = recv(fd, buffer, sizeof(buffer), 0);
        if(bytes_received<=0) break;
        body.insert(body.end(), buffer, buffer+bytes_received);
    }
    parse_body(body);

}

void HttpRequest::parse_headers(const std::string& header_block){
    int index = header_block.find(" ");
    headers_["method"] = header_block.substr(0, index);
    int path_len = header_block.find(" ", index+1)-(index+1);
    headers_["path"] = header_block.substr(index+1, path_len);


    int boundary_start = header_block.find("boundary");
    if(boundary_start!=std::string::npos){
        int boundary_end = header_block.find("\r\n", boundary_start);
        headers_["boundary"] = "--"+header_block.substr(boundary_start+9, boundary_end-boundary_start-9);
    }

    int cl_start = header_block.find("Content-Length");
    if(cl_start!=std::string::npos){
        int cl_end = header_block.find("\r\n", cl_start);
        int cl_len = cl_end - cl_start - 16;
        headers_["length"] = header_block.substr(cl_start+16, cl_len);
    }
}

void HttpRequest::parse_body(const std::vector<unsigned char>& body_block){
    auto pipeline_start = std::search(body_block.begin(), body_block.end(), "\r\n\r\n", "\r\n\r\n"+4);
    auto b = headers_["boundary"];
    auto pipeline_end = std::search(pipeline_start+4, body_block.end(), "\r\n", "\r\n"+2);
    pipeline_ = std::string(pipeline_start+4, pipeline_end);

    auto ct_start = std::search(body_block.begin(), body_block.end(), "Content-Type", "Content-Type"+12);
    auto ct_end = std::search(ct_start, body_block.end(), "\r\n\r\n", "\r\n\r\n"+4);
    auto ct_space = std::search(ct_start, ct_end, " ", " "+1);
    auto ct_slash = std::search(ct_start, ct_end, "/", "/"+1);
    file_type_ = std::string(ct_space+1, ct_slash);
    file_format_ = std::string(ct_slash+1, ct_end);
    auto bytes_start = ct_end+4;
    auto bytes_end = std::search(bytes_start, body_block.end(), b.begin(), b.end());
    file_bytes_ = std::vector<unsigned char>(bytes_start, bytes_end);
}

std::string HttpRequest::pipeline() const{
    return pipeline_;
}

std::string HttpRequest::file_type() const{
    return file_type_;
}
std::string HttpRequest::file_format() const{
    return file_format_;
}

std::string HttpRequest::headers(const std::string& key){
    auto it = headers_.find(key);
    if (it == headers_.end()) throw std::runtime_error("Missing header: " + key);
    return it->second;
}

const std::vector<unsigned char>& HttpRequest::file_bytes() const{
    return file_bytes_;
}

bool HttpRequest::isValid() const{
    return isValid_;
}
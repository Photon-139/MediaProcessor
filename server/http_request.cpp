#include "http_request.hpp"
#include "tcp_connection.hpp"
#include <sys/socket.h>
#include <cstring>
#include <algorithm>
#include <spdlog/spdlog.h>

void HttpRequest::feed(const unsigned char* buff, size_t length){
    if(state_==ParseState::COMPLETE || state_==ParseState::ERROR_STATE) return;
    buffer_.insert(buffer_.end(), buff, buff+length);
    
    while(true){
        switch(state_){
            case ParseState::READING_HEADERS:{
                auto header_boundary = std::search(buffer_.begin(), buffer_.end(), "\r\n\r\n", "\r\n\r\n"+4);
                if(header_boundary==buffer_.end()) return;
                std::string_view header_view(
                    reinterpret_cast<const char*>(buffer_.data()),
                    std::distance(buffer_.begin(), header_boundary)
                );
                parse_headers(header_view);
                body_start_offset_ = std::distance(buffer_.begin(), header_boundary)+4;
                if(headers_["method"]=="OPTIONS"){
                    state_ = ParseState::COMPLETE;
                    return;
                }
                state_ = ParseState::READING_BODY;
                break;
            }
            case ParseState::READING_BODY:{
                const std::string& len_str = headers_["length"];
                int cl = 0;
                auto [ptr, ec] = std::from_chars(len_str.data(), len_str.data() + len_str.size(), cl);
                if (ec != std::errc{} || cl < 0) {
                    state_ = ParseState::ERROR_STATE;
                    return; // or however you currently bail into error state
                }
                if(buffer_.size()-body_start_offset_<static_cast<size_t>(cl)) return;
                parse_body(buffer_.begin()+body_start_offset_, buffer_.begin()+body_start_offset_+cl);
                state_ = ParseState::COMPLETE;
                return;
            }
            case ParseState::COMPLETE:
            case ParseState::ERROR_STATE:
                return;
        }
    }
}

void HttpRequest::parse_headers(std::string_view header_block){
    int index = header_block.find(" ");
    headers_["method"] = header_block.substr(0, index);
    int path_len = header_block.find(" ", index+1)-(index+1);
    headers_["path"] = header_block.substr(index+1, path_len);


    auto boundary_start = header_block.find("boundary");
    if(boundary_start!=std::string_view::npos){
        int boundary_end = header_block.find("\r\n", boundary_start);
        std::string boundary = "--";
        boundary.append(header_block.substr(boundary_start+9, boundary_end-boundary_start-9));
        headers_["boundary"] = std::move(boundary);
    }

    auto cl_start = header_block.find("Content-Length");
    if(cl_start!=std::string_view::npos){
        /* For review
            int cl_end = header_block.find("\r\n", cl_start);
            int cl_len = cl_end - cl_start - 16;
            headers_["length"] = header_block.substr(cl_start+16, cl_len);
        */
       auto colon_pos = header_block.find(":", cl_start);
       auto cl_end = header_block.find("\r\n", cl_start);
       if(colon_pos!=std::string_view::npos && cl_end!=std::string_view::npos){
            auto val_start = colon_pos+1;
            while(val_start<cl_end && header_block[val_start]==' '){
                val_start++;
            }
            headers_["length"] = header_block.substr(val_start, cl_end-val_start);
       }
    }
}

void HttpRequest::parse_body(std::vector<unsigned char>::const_iterator body_start, std::vector<unsigned char>::const_iterator body_end){
    auto pipeline_start = std::search(body_start, body_end, "\r\n\r\n", "\r\n\r\n"+4);
    auto b = headers_["boundary"];
    auto pipeline_end = std::search(pipeline_start+4, body_end, "\r\n", "\r\n"+2);
    pipeline_ = std::string(pipeline_start+4, pipeline_end);

    auto ct_start = std::search(body_start, body_end, "Content-Type", "Content-Type"+12);
    auto ct_end = std::search(ct_start, body_end, "\r\n\r\n", "\r\n\r\n"+4);
    auto ct_space = std::search(ct_start, ct_end, " ", " "+1);
    auto ct_slash = std::search(ct_start, ct_end, "/", "/"+1);
    file_type_ = std::string(ct_space+1, ct_slash);
    file_format_ = std::string(ct_slash+1, ct_end);
    auto bytes_start = ct_end+4;
    auto bytes_end = std::search(bytes_start, body_end, b.begin(), b.end());
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

void HttpRequest::reset() {
    buffer_.clear();
    state_ = ParseState::READING_HEADERS;
    file_bytes_.clear();
    pipeline_.clear();
    file_type_.clear();
    file_format_.clear();
    headers_.clear();
    body_start_offset_ = 0;
}

ParseState HttpRequest::state() const{
    return state_;
}
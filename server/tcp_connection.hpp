#pragma once
class TCPConnection{
    public:
        TCPConnection(int fd);
        ~TCPConnection();
        int fd() const;
    private:
        int fd_;
};
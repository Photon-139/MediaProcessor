#pragma once
class TCPConnection{
    public:
        explicit TCPConnection(int fd);
        ~TCPConnection();
        int fd() const;
        TCPConnection(const TCPConnection&) = delete;
        TCPConnection& operator=(const TCPConnection&) = delete;
        TCPConnection(TCPConnection&& other) noexcept;
        TCPConnection& operator=(TCPConnection&& other) noexcept;
    private:
        int fd_;
};
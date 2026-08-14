//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/address.h>

class Socket {
public:
    Socket() = default;
    explicit Socket(int type);

    ~Socket();

    void setFd(int fd) {
        fd_ = fd;
    }
    [[nodiscard]] const int& getFd() const {
        return fd_;
    }

    void bind(const Dream::Address& address);
    void listen();
    [[nodiscard]] int accept(Dream::Address& peerAddress);
    void shutdown() const;

    void setTcpNoDelay(bool on) const;
    void setReUseAddr(bool on) const;
    void setReUsePort(bool on) const;
    void setKeepAlive(bool on) const;

private:
    int fd_ = 0;
};

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/address.h>

class Socket {
public:
    explicit Socket(int type);
    ~Socket();

    [[nodiscard]] const int& getFd() const {
        return fd_;
    }

    void bind(const Dream::Address& address);
    void listen();

    void setTcpNoDelay(bool on) const;
    void setReUseAddr(bool on) const;
    void setReUsePort(bool on) const;
    void setKeepAlive(bool on) const;

private:
    int fd_ = 0;
};

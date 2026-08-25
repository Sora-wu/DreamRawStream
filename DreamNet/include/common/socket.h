//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/address.h>

namespace Dream::detail {
    class Socket {
    public:
        Socket() = default;
        explicit Socket(int type);
        Socket(const Socket& socket) = delete;
        Socket(Socket&& socket) noexcept;
        Socket& operator=(Socket&& socket) noexcept;
        ~Socket();

        void setFd(int fd) {
            fd_ = fd;
        }

        [[nodiscard]] const int& getFd() const {
            return fd_;
        }

        void bind(const Address& address);
        void listen();
        [[nodiscard]] int connect(const Address& address) const;
        [[nodiscard]] int accept(Address& peerAddress);
        void shutdown() const;

        void setTcpNoDelay(bool on) const;
        void setReUseAddr(bool on) const;
        void setReUsePort(bool on) const;
        void setKeepAlive(bool on) const;

    private:
        int fd_ = -1;
    };
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <common/socket.h>
#include <common/define.h>

#include <unistd.h>
#include <netinet/tcp.h>

using namespace Dream::detail;

Socket::Socket(int type) {
    fd_ = socket(AF_INET, type, 0);
    if (fd_ < 0) {
        LOG_FATAL("socket fail! {}", strerror(errno));
    }
}

Socket::Socket(Socket&& socket) noexcept {
    fd_ = socket.fd_;
    socket.fd_ = -1;
}

Socket& Socket::operator=(Socket&& socket) noexcept {
    ::close(fd_);
    fd_ = socket.fd_;
    socket.fd_ = -1;
    return *this;
}

Socket::~Socket() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

void Socket::bind(const Address& address) {
    sockaddr_in addr = address.getAddr();
    if (::bind(fd_, (sockaddr*)&addr, sizeof(sockaddr_in))) {
        LOG_FATAL("bind error, fd: {}, {}", fd_, strerror(errno));
    }
}

void Socket::listen() {
    if (::listen(fd_, SOMAXCONN) < 0) {
        LOG_FATAL("listen error, fd: {}, {}", fd_, strerror(errno));
    }
}

int Socket::connect(const Address& address) const {
    sockaddr_in addr = address.getAddr();
    return ::connect(fd_, (sockaddr*)&addr, sizeof(sockaddr_in));
}

int Socket::accept(Address& peerAddress) {
    sockaddr_in addr{};
    socklen_t addrlen = sizeof(addr);
    int connfd = ::accept4(fd_, (sockaddr*)&addr, &addrlen, SOCK_CLOEXEC | SOCK_NONBLOCK);
    if (connfd < 0) {
        LOG_ERROR("accept error, fd: {}, {}", fd_, strerror(errno));
    }

    peerAddress.setAddr(addr);
    return connfd;
}

void Socket::shutdown() const {
    if (::shutdown(fd_, SHUT_WR) < 0) {
        LOG_ERROR("shutdown error");
    }
}

void Socket::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

void Socket::setTcpNoDelay(bool on) const {
    int optval = on ? 1 : 0;
    setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReUseAddr(bool on) const {
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setReUsePort(bool on) const {
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on) const {
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

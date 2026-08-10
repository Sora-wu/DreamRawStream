//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <common/socket.h>
#include <common/define.h>

#include <unistd.h>
#include <netinet/tcp.h>

using namespace Dream;

Socket::Socket(int type) {
    int fd = socket(AF_INET, type, 0);
    if (fd < 0) {
        LOG_FATAL("socket fail! {}", strerror(errno));
    }
}

Socket::~Socket() {
    close(fd_);
    fd_ = -1;
}

void Socket::bind(const Address& address) {
    if (::bind(fd_, (sockaddr*)address.getAddr(), sizeof(sockaddr_in))) {
        LOG_FATAL("bind error, fd: {}, {}", fd_, strerror(errno));
    }
}

void Socket::listen() {
    if (::listen(fd_, SOMAXCONN) < 0) {
        LOG_FATAL("listen error, fd: {}, {}", fd_, strerror(errno));
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

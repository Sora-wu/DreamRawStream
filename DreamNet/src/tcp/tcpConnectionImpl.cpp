//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/tcpConnectionImpl.h>
#include <tcp/eventLoopImpl.h>
#include <common/log.hpp>

#include <unistd.h>

using namespace Dream::detail;

namespace {
    constexpr int BUFFER_SIZE = 1024 * 64;
}

TcpConnectionImpl::TcpConnectionImpl(TcpConnection* conn, EventLoop* loop, const std::string& name, int fd,
                                     const Address& localAddress, const Address& remoteAddress) :
    conn_(conn),
    name_(name),
    loop_(loop),
    localAddress_(localAddress),
    remoteAddress_(remoteAddress),
    channel_(std::make_unique<Channel>(loop, fd)),
    inBuffer_(BUFFER_SIZE), outBuffer_(BUFFER_SIZE) {
    socket_.setFd(fd);
}

Dream::Address TcpConnectionImpl::getLocalAddress() const {
    return localAddress_;
}

Dream::Address TcpConnectionImpl::getRemoteAddress() const {
    return remoteAddress_;
}

std::string TcpConnectionImpl::getName() const {
    return name_;
}

void TcpConnectionImpl::connectEstablished() {
    state_ = ConnectionState::CONNECTED;

    channel_->setOnReadEvent([this] { handleRead(); });
    channel_->setOnWriteEvent([this] { handleWrite(); });
    channel_->setOnCloseEvent([this] { handleClose(); });
    channel_->setOnErrorEvent([this] { handleError(); });
    channel_->enableReading();

    if (connectionCallback_) {
        connectionCallback_(conn_);
    }
}

void TcpConnectionImpl::connectDestroyed() {
    if (state_ == ConnectionState::CONNECTED) {
        state_ = ConnectionState::DISCONNECTED;
        channel_->disableReading();
    }
}

bool TcpConnectionImpl::isConnected() const {
    return state_ == ConnectionState::CONNECTED;
}

void TcpConnectionImpl::send(Buffer& buffer) {
    loop_->runInLoop([&] { sendInLoop(buffer); });
}

void TcpConnectionImpl::shutdown() {
    if (state_ == ConnectionState::CONNECTED) {
        state_ = ConnectionState::DISCONNECTED;
        loop_->runInLoop([this] {
            if (!channel_->isWriting()) {
                socket_.shutdown();
            }
        });
    }
}

void TcpConnectionImpl::setConnectionCallback(ConnectionCallback cb) {
    connectionCallback_ = std::move(cb);
}

void TcpConnectionImpl::setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
}

void TcpConnectionImpl::setCloseCallback(CloseCallback cb) {
    closeCallback_ = std::move(cb);
}

void TcpConnectionImpl::sendInLoop(Buffer& buffer) {
    int fd = socket_.getFd();

    if (!channel_->isWriting()) {
        const std::span<const char> data = buffer.peek();
        ssize_t n = write(fd, data.data(), data.size());
        if (n < 0) {
            LOG_ERROR("write error: {}", strerror(errno));
            return;
        }

        buffer.consume(n);
    }

    if (buffer.readableSize() > 0) {
        outBuffer_.append(buffer);
    }

    if (!channel_->isWriting() && buffer.readableSize() > 0) {
        channel_->enableWriting();
    }
}

void TcpConnectionImpl::handleRead() {
    char buff[BUFFER_SIZE]{};
    int fd = socket_.getFd();

    ssize_t n = recv(fd, buff, BUFFER_SIZE, 0);
    if (n == 0) {
        handleClose();
        return;
    }
    if (n < 0) {
        handleError();
        return;
    }

    inBuffer_.write(std::string(buff, n));
    if (messageCallback_) {
        messageCallback_(conn_, inBuffer_);
    }
}

void TcpConnectionImpl::handleWrite() {
    int fd = socket_.getFd();
    if (!channel_->isWriting()) {
        LOG_ERROR("this connection {} is disable writing!", fd);
        return;
    }

    const std::span<const char> data = outBuffer_.peek();
    ssize_t n = write(fd, data.data(), data.size());
    if (n < 0) {
        LOG_ERROR("write error: {}", strerror(errno));
        return;
    }

    outBuffer_.consume(n);
    if (outBuffer_.readableSize() == 0) {
        channel_->disableWriting();
    }
}

void TcpConnectionImpl::handleClose() {
    LOG_ERROR("client closed connection");
    state_ = ConnectionState::DISCONNECTED;

    if (closeCallback_) {
        closeCallback_(conn_);
    }
}

void TcpConnectionImpl::handleError() const {
    int optval{};
    socklen_t optlen = sizeof(optval);
    int fd = socket_.getFd();
    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        LOG_ERROR("getsockopt error: {}", strerror(errno));
    }

    LOG_ERROR("client error: SO_ERROR-{}", optval);
}

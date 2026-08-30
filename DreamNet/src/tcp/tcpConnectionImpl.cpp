//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/tcpConnectionImpl.h>
#include <tcp/eventLoopImpl.h>
#include <common/log.hpp>

#include <unistd.h>

#include "DreamNet/tcpConnection.h"

using namespace Dream::detail;

namespace {
    constexpr uint32_t BUFFER_SIZE = 1024 * 64;
    constexpr uint32_t HIGH_WATER_MARK_SIZE = 1024 * 1024;         // 1M
}

TcpConnectionImpl::TcpConnectionImpl(TcpConnection* conn, EventLoopImpl* loop, const std::string& name, int fd,
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

    channel_->tie(conn_->shared_from_this());
    channel_->setOnReadEvent([this] { handleRead(); });
    channel_->setOnWriteEvent([this] { handleWrite(); });
    channel_->setOnCloseEvent([this] { handleClose(); });
    channel_->setOnErrorEvent([this] { handleError(); });
    channel_->enableReading();

    if (connectionCallback_) {
        connectionCallback_(conn_->shared_from_this());
    }
}

void TcpConnectionImpl::connectDestroyed() {
    state_ = ConnectionState::DISCONNECTED;
    channel_->disableAll();
}

bool TcpConnectionImpl::isConnected() const {
    return state_ == ConnectionState::CONNECTED;
}

void TcpConnectionImpl::send(const Buffer& buffer, std::shared_ptr<const TcpConnection> self) {
    loop_->runInLoop([this, self = std::move(self), buf = std::move(buffer)]() mutable {
        (void)self; // 仅持有owner引用，保证this在functor执行前不被析构，下同
        sendInLoop(buf);
    });
}

void TcpConnectionImpl::send(const std::span<char>& data, std::shared_ptr<const TcpConnection> self) {
    Buffer buf;
    buf.write(data);

    loop_->runInLoop([this, self = std::move(self), buf = std::move(buf)]() mutable {
        (void)self;
        sendInLoop(buf);
    });
}

void TcpConnectionImpl::send(const char* data, size_t size, std::shared_ptr<const TcpConnection> self) {
    std::span buffer(data, size);
    Buffer buf;
    buf.write(buffer);

    loop_->runInLoop([this, self = std::move(self), buf = std::move(buf)]() mutable {
        (void)self;
        sendInLoop(buf);
    });
}

void TcpConnectionImpl::shutdown(std::shared_ptr<const TcpConnection> self) {
    if (state_ == ConnectionState::CONNECTED) {
        state_ = ConnectionState::DISCONNECTING;
        loop_->runInLoop([this, self = std::move(self)] {
            (void)self;
            if (!channel_->isWriting()) {
                socket_.shutdown();
            }
            else {
                shutdownAfterWrite_ = true;
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

void TcpConnectionImpl::setHighWaterMarkCallback(HighWaterMarkCallback cb) {
    highWaterMarkCallback_ = std::move(cb);
}

void TcpConnectionImpl::setWriteCompleteCallback(WriteCompleteCallback cb) {
    writeCompleteCallback_ = std::move(cb);
}

void TcpConnectionImpl::sendInLoop(Buffer& buffer) {
    if (state_ != ConnectionState::CONNECTED) {
        return;
    }

    int fd = socket_.getFd();

    if (!channel_->isWriting()) {
        const std::span<const char> data = buffer.peek();
        ssize_t n = write(fd, data.data(), data.size());
        if (n > 0) {
            buffer.consume(n);
        }
        else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("write error: {}", strerror(errno));
            return;
        }
    }

    if (buffer.readableSize() > 0) {
        if (outBuffer_.readableSize() < HIGH_WATER_MARK_SIZE && outBuffer_.readableSize() + buffer.readableSize() >= HIGH_WATER_MARK_SIZE
            && highWaterMarkCallback_) {
            // 触发高水位预警
            LOG_WARN("trigger highWaterMark!");
            highWaterMarkCallback_(conn_->shared_from_this());
        }

        outBuffer_.append(buffer);
        buffer.consume(buffer.readableSize()); // 消费源 buffer，避免下次重复发送
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
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
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            handleError();
        }
        return;
    }

    inBuffer_.write(std::span(buff, n));
    if (messageCallback_) {
        const uint32_t consume = messageCallback_(conn_->shared_from_this(), inBuffer_);
        inBuffer_.consume(consume);
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
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return; // 内核发送缓冲满，保持写使能，等待下次可写事件
        }
        LOG_ERROR("write error: {}", strerror(errno));
        return;
    }

    outBuffer_.consume(n);
    if (outBuffer_.readableSize() == 0) {
        if (writeCompleteCallback_) {
            writeCompleteCallback_(conn_->shared_from_this());
        }

        if (shutdownAfterWrite_) {
            shutdownAfterWrite_ = false;
            socket_.shutdown();
        }
        channel_->disableWriting();
    }
}

void TcpConnectionImpl::handleClose() {
    if (state_ == ConnectionState::DISCONNECTED) {
        return;
    }

    LOG_INFO("peer closed connection");
    state_ = ConnectionState::DISCONNECTED;

    if (closeCallback_) {
        closeCallback_(conn_->shared_from_this());
    }
}

void TcpConnectionImpl::handleError() {
    if (state_ == ConnectionState::DISCONNECTED) {
        return;
    }

    int optval{};
    socklen_t optlen = sizeof(optval);
    int fd = socket_.getFd();
    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        LOG_ERROR("getsockopt error: {}", strerror(errno));
    }

    LOG_ERROR("client error: SO_ERROR-{}", optval);
    handleClose();
}

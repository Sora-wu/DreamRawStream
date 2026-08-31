//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/tcpClientImpl.h>
#include <tcp/connector.h>
#include <tcp/eventLoopImpl.h>
#include <DreamNet/tcpConnection.h>
#include <tcp/tcpConnectionImpl.h>
#include <DreamNet/eventLoop.h>
#include <common/log.hpp>

#include <format>
#include <unistd.h>

using namespace Dream;

detail::TcpClientImpl::TcpClientImpl(EventLoop* loop, const Address& addr) :
    loop_(loop),
    connector_(std::make_shared<Connector>(EventLoopImpl::from(loop), addr)) {
    connector_->setNewConnectionCallback([this](int fd, const Address& addr) {
        onNewConnection(fd, addr);
    });
}

void detail::TcpClientImpl::connect() {
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf] {
        if (auto self = weakSelf.lock()) {
            if (self->connection_) {
                LOG_WARN("connection has been started");
                return;
            }

            self->manualDisconnect_ = false;
            self->connector_->start();
        }
    });
}

void detail::TcpClientImpl::disconnect() {
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf] {
        if (auto self = weakSelf.lock()) {
            // 如果已经连接上，断开connection_即可
            if (self->connection_) {
                self->manualDisconnect_ = true;
                self->connection_->shutdown();
                return;
            }

            self->connector_->stop();
        }
    });
}

void detail::TcpClientImpl::close() {
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf] {
        if (auto self = weakSelf.lock()) {
            self->manualDisconnect_ = true;
            self->connection_->close();
            self->connection_.reset();
            self->connector_->stop();
        }
    });
}

void detail::TcpClientImpl::setRetryInterval(uint32_t retryInterval) const {
    connector_->setRetryInterval(retryInterval);
}

void detail::TcpClientImpl::send(const Buffer& buffer) const {
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf, buffer = std::move(buffer)] {
        if (auto self = weakSelf.lock()) {
            if (!self->connection_) {
                LOG_ERROR("not connected, drop: {}", buffer.getView());
                return;
            }
            self->connection_->send(buffer);
        }
    });
}

void detail::TcpClientImpl::send(const std::span<char>& data) const {
    Buffer buffer;
    buffer.write(data);

    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf, buffer = std::move(buffer)] {
        if (auto self = weakSelf.lock()) {
            if (!self->connection_) {
                LOG_ERROR("not connected, drop: {}", buffer.getView());
                return;
            }
            self->connection_->send(buffer);
        }
    });
}

void detail::TcpClientImpl::send(const char* data, uint32_t size) const {
    if (!data || size == 0) {
        return;
    }

    Buffer buffer;
    buffer.write(std::span(data, size));
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf, buffer = std::move(buffer)] {
        if (auto self = weakSelf.lock()) {
            if (!self->connection_) {
                LOG_ERROR("not connected, drop: {}", buffer.getView());
                return;
            }
            self->connection_->send(buffer);
        }
    });
}

void detail::TcpClientImpl::setConnectionCallback(ConnectionCallback cb) {
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf, cb = std::move(cb)] {
        if (auto self = weakSelf.lock()) {
            self->connectionCallback_ = std::move(cb);
        }
    });
}

void detail::TcpClientImpl::setMessageCallback(MessageCallback cb) {
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf, cb = std::move(cb)] {
        if (auto self = weakSelf.lock()) {
            self->messageCallback_ = std::move(cb);
        }
    });
}

void detail::TcpClientImpl::setWriteCompleteCallback(WriteCompleteCallback cb) {
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf, cb = std::move(cb)] {
        if (auto self = weakSelf.lock()) {
            self->writeCompleteCallback_ = std::move(cb);
        }
    });
}

void detail::TcpClientImpl::setHighWaterMarkCallback(HighWaterMarkCallback cb) {
    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf, cb = std::move(cb)] {
        if (auto self = weakSelf.lock()) {
            self->highWaterMarkCallback_ = std::move(cb);
        }
    });
}

void detail::TcpClientImpl::onNewConnection(int fd, const Address& peerAddr) {
    LOG_INFO("new client connected: {}:{}", peerAddr.getIP(), peerAddr.getPort());

    sockaddr_in localAddr{};
    socklen_t localAddrLen = sizeof(localAddr);
    if (getsockname(fd, (sockaddr*)&localAddr, &localAddrLen) < 0) {
        ::close(fd);
        LOG_ERROR("getsockname failed: {}", strerror(errno));
        return;
    }

    Address local{};
    local.setAddr(localAddr);
    const std::string connName = std::format("connection-{}:{}", local.getIP(), local.getPort());
    connection_ = std::make_shared<TcpConnection>(loop_, connName, fd, local, peerAddr);
    connection_->setConnectionCallback(connectionCallback_);
    connection_->setMessageCallback(messageCallback_);
    connection_->setWriteCompleteCallback(writeCompleteCallback_);
    std::weak_ptr weakSelf = shared_from_this();
    connection_->setCloseCallback([weakSelf](TcpConnectionPtr connection) {
        if (auto self = weakSelf.lock()) {
            self->onConnectionClose(connection);
        }
    });

    loop_->runInLoop([weakSelf] {
        if (auto self = weakSelf.lock()) {
            self->connection_->connectEstablished();
        }
    });
}

void detail::TcpClientImpl::onConnectionClose(TcpConnectionPtr connection) {
    connection->getLoop()->runInLoop([connection] {
        connection->connectDestroyed();
    });

    std::weak_ptr weakSelf = shared_from_this();
    loop_->runInLoop([weakSelf, connection] {
        if (auto self = weakSelf.lock()) {
            std::string name = connection->getName();
            const Address& addr = connection->getRemoteAddress();
            LOG_INFO("connection closed, connection name: {}, {}:{}", name, addr.getIP(), addr.getPort());

            self->connection_.reset();

            // 如果是手动disconnect，那么不需要重连
            if (self->connector_ && !self->manualDisconnect_) {
                // 若服务端主动关闭，启动重启流程
                self->connector_->start();
            }
        }
    });
}

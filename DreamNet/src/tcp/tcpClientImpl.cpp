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
    connector_(std::make_unique<Connector>(EventLoopImpl::from(loop), addr)) {
    connector_->setNewConnectionCallback([this](int fd, const Address& addr) { onNewConnection(fd, addr); });
}

void detail::TcpClientImpl::connect() {
    loop_->runInLoop([this] {
        if (connection_) {
            LOG_WARN("connection has been started");
            return;
        }

        manualDisconnect_ = false;
        connector_->start();
    });
}

void detail::TcpClientImpl::disconnect() {
    loop_->runInLoop([this] {
        // 如果已经连接上，断开connection_即可
        if (connection_) {
            manualDisconnect_ = true;
            connection_->shutdown();
            return;
        }

        connector_->stop();
    });
}

void detail::TcpClientImpl::setRetryInterval(uint32_t retryInterval) const {
    connector_->setRetryInterval(retryInterval);
}

void detail::TcpClientImpl::send(const Buffer& buffer) const {
    loop_->runInLoop([this, buffer = std::move(buffer)] {
        if (!connection_) {
            LOG_ERROR("not connected, drop: {}", buffer.getView());
            return;
        }
        connection_->send(buffer);
    });
}

void detail::TcpClientImpl::send(const std::span<char>& data) const {
    Buffer buffer;
    buffer.write(data);

    loop_->runInLoop([this, buffer = std::move(buffer)] {
        if (!connection_) {
            LOG_ERROR("not connected, drop: {}", buffer.getView());
            return;
        }
        connection_->send(buffer);
    });
}

void detail::TcpClientImpl::send(const char* data, uint32_t size) const {
    if (!data || size == 0) {
        return;
    }

    Buffer buffer;
    buffer.write(std::span(data, size));
    loop_->runInLoop([this, buffer = std::move(buffer)] {
        if (!connection_) {
            LOG_ERROR("not connected, drop: {}", buffer.getView());
            return;
        }
        connection_->send(buffer);
    });
}

void detail::TcpClientImpl::setConnectionCallback(ConnectionCallback cb) {
    loop_->runInLoop([this, cb = std::move(cb)] {
        connectionCallback_ = std::move(cb);
    });
}

void detail::TcpClientImpl::setMessageCallback(MessageCallback cb) {
    loop_->runInLoop([this, cb = std::move(cb)] {
        messageCallback_ = std::move(cb);
    });
}

void detail::TcpClientImpl::setWriteCompleteCallback(WriteCompleteCallback cb) {
    loop_->runInLoop([this, cb = std::move(cb)] {
        writeCompleteCallback_ = std::move(cb);
    });
}

void detail::TcpClientImpl::setHighWaterMarkCallback(HighWaterMarkCallback cb) {
    loop_->runInLoop([this, cb = std::move(cb)] {
        highWaterMarkCallback_ = std::move(cb);
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
    connection_->setCloseCallback([this](TcpConnectionPtr connection) { onConnectionClose(connection); });

    loop_->runInLoop([this] {
        connection_->connectEstablished();
    });
}

void detail::TcpClientImpl::onConnectionClose(TcpConnectionPtr connection) {
    connection->getLoop()->runInLoop([connection] {
        connection->connectDestroyed();
    });

    loop_->runInLoop([connection, this] {
        std::string name = connection->getName();
        const Address& addr = connection->getRemoteAddress();
        LOG_INFO("connection closed, connection name: {}, {}:{}", name, addr.getIP(), addr.getPort());

        connection_.reset();

        // 如果是手动disconnect，那么不需要重连
        if (connector_ && !manualDisconnect_) {
            // 若服务端主动关闭，启动重启流程
            connector_->start();
        }
    });
}

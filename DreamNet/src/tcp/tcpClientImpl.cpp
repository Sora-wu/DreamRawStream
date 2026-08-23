//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/tcpClientImpl.h>
#include <tcp/connector.h>
#include <tcp/eventLoopImpl.h>
#include <tcp/channel.h>
#include <DreamNet/tcpConnection.h>
#include <DreamNet/eventLoop.h>
#include <common/log.hpp>

#include <format>

using namespace Dream;

detail::TcpClientImpl::TcpClientImpl(EventLoop* loop, const Address& addr) :
    loop_(loop),
    connector_(std::make_unique<Connector>(EventLoopImpl::from(loop), addr)) {
    connector_->setNewConnectionCallback([this](int fd, const Address& addr) {
        onNewConnection(fd, addr);
    });
}

void detail::TcpClientImpl::connect() const {
    loop_->runInLoop([this] {
        connector_->start();
    });
}

void detail::TcpClientImpl::disconnect() {
    // 如果已经连接上，断开connection_即可
    if (connection_) {
        connection_->shutdown();
        connection_.reset();
        return;
    }

    connector_->stop();
}

void detail::TcpClientImpl::setRetryInterval(uint32_t retryInterval) const {
    connector_->setRetryInterval(retryInterval);
}

void detail::TcpClientImpl::send(const Buffer& buffer) const {
    if (!connection_) {
        LOG_ERROR("not connected, drop: {}", buffer.getView());
        return;
    }

    connection_->send(buffer);
}

void detail::TcpClientImpl::send(const std::span<char>& buffer) const {
    if (!connection_) {
        LOG_ERROR("not connected, drop: {}", std::string_view(buffer.data(), buffer.size()));
        return;
    }

    connection_->send(buffer);
}

void detail::TcpClientImpl::send(const char* data, uint32_t size) const {
    if (!connection_) {
        LOG_ERROR("not connected, drop: {}", std::string_view(data, size));
        return;
    }

    connection_->send(data, size);
}

void detail::TcpClientImpl::setConnectionCallback(ConnectionCallback cb) {
    connectionCallback_ = std::move(cb);
}

void detail::TcpClientImpl::setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
}

void detail::TcpClientImpl::setWriteCompleteCallback(WriteCompleteCallback cb) {
    writeCompleteCallback_ = std::move(cb);
}

void detail::TcpClientImpl::onNewConnection(int fd, const Address& peerAddr) {
    LOG_INFO("new client connected: {}:{}", peerAddr.getIP(), peerAddr.getPort());

    sockaddr_in localAddr{};
    socklen_t localAddrLen = sizeof(localAddr);
    if (getsockname(fd, (sockaddr*)&localAddr, &localAddrLen) < 0) {
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
    connection_->setCloseCallback([this](TcpConnection* connection) { onConnectionClose(connection); });

    loop_->runInLoop([this] {
        connection_->connectEstablished();
    });
}

void detail::TcpClientImpl::onConnectionClose(TcpConnection* connection) {
    connection->getLoop()->runInLoop([connection] {
        connection->connectDestroyed();
    });

    loop_->runInLoop([connection, this] {
        std::string name = connection->getName();
        const Address& addr = connection->getRemoteAddress();
        LOG_INFO("connection closed, connection name: {}, {}:{}", name, addr.getIP(), addr.getPort());

        connection_.reset();
    });
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/tcpServerImpl.h>
#include <tcp/eventLoopImpl.h>
#include <tcp/acceptor.h>
#include <tcp/channel.h>
#include <tcp/threadPool.h>
#include <DreamNet/eventLoop.h>
#include <DreamNet/tcpConnection.h>
#include <common/log.hpp>

#include <format>
#include <ranges>

using namespace Dream;

detail::TcpServerImpl::TcpServerImpl(EventLoop* loop, const Address& addr) :
    loop_(loop),
    acceptor_(std::make_unique<Acceptor>(EventLoopImpl::from(loop), addr)),
    threadPool_(std::make_unique<ThreadPool>(loop)) {
    acceptor_->setNewConnectionFunc([this](int fd, const Address& addr) {
        onNewConnection(fd, addr);
    });

    // 高水位回调默认关闭对方连接
    highWaterMarkCallback_ = [this](TcpConnection* conn) {
        onConnectionClose(conn);
    };
}

detail::TcpServerImpl::~TcpServerImpl() {
    std::unique_lock lock(smutx_);
    for (auto& conn : connections_ | std::views::values) {
        conn->getLoop()->runInLoop([conn] {
            conn->connectDestroyed();
        });
    }
}

void detail::TcpServerImpl::setThreadNum(uint32_t threadCount) const {
    threadPool_->setThreadNum(threadCount);
}

void detail::TcpServerImpl::start() {
    if (!isRunning_) {
        threadPool_->start();
        loop_->runInLoop([this] {
            acceptor_->listen();
        });
        isRunning_ = true;
    }
}

void detail::TcpServerImpl::forEachConnect(const std::function<void(TcpConnection*)>& callback) {
    std::vector<TcpConnectionPtr> snapshotConn;

    {
        std::shared_lock lock(smutx_);
        for (auto& conn : connections_ | std::views::values) {
            snapshotConn.push_back(conn);
        }
    }

    for (auto& conn : snapshotConn) {
        callback(conn.get());
    }
}

uint32_t detail::TcpServerImpl::getConnectionCount() {
    std::shared_lock lock(smutx_);
    return connections_.size();
}

void detail::TcpServerImpl::sendBroadcast(const Buffer& buffer) {
    forEachConnect([&](TcpConnection* conn) {
        conn->send(buffer);
    });
}

void detail::TcpServerImpl::sendBroadcast(const std::span<char>& buffer) {
    forEachConnect([&](TcpConnection* conn) {
        conn->send(buffer);
    });
}

void detail::TcpServerImpl::sendBroadcast(const char* data, uint32_t size) {
    forEachConnect([&](TcpConnection* conn) {
        conn->send(data, size);
    });
}

void detail::TcpServerImpl::setConnectionCallback(ConnectionCallback cb) {
    connectionCallback_ = std::move(cb);
}

void detail::TcpServerImpl::setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
}

void detail::TcpServerImpl::setHighWaterMarkCallback(HighWaterMarkCallback cb) {
    highWaterMarkCallback_ = std::move(cb);
}

void detail::TcpServerImpl::setWriteCompleteCallback(WriteCompleteCallback cb) {
    writeCompleteCallback_ = std::move(cb);
}

void detail::TcpServerImpl::onNewConnection(int fd, const Address& peerAddr) {
    EventLoop* loop = threadPool_->getNextLoop();
    LOG_INFO("new client connected: {}:{}", peerAddr.getIP(), peerAddr.getPort());

    sockaddr_in localAddr{};
    socklen_t localAddrLen = sizeof(localAddr);
    if (getsockname(fd, (sockaddr*)&localAddr, &localAddrLen) < 0) {
        LOG_ERROR("getsockname failed: {}", strerror(errno));
        return;
    }

    Address local{};
    local.setAddr(localAddr);
    const std::string connName = std::format("connection-{}", connectionID++);
    TcpConnectionPtr connection = std::make_shared<TcpConnection>(loop, connName, fd, local, peerAddr);
    connection->setConnectionCallback(connectionCallback_);
    connection->setMessageCallback(messageCallback_);
    connection->setHighWaterMarkCallback(highWaterMarkCallback_);
    connection->setCloseCallback([this](TcpConnection* conn) { onConnectionClose(conn); });
    {
        std::unique_lock lock(smutx_);
        connections_[connName] = connection;
    }

    loop->runInLoop([connection] {
        connection->connectEstablished();
    });
}

void detail::TcpServerImpl::onConnectionClose(TcpConnection* connection) {
    connection->getLoop()->runInLoop([connection] {
        connection->connectDestroyed();
    });

    loop_->runInLoop([this, connection] {
        std::string name = connection->getName();
        const Address& addr = connection->getRemoteAddress();
        LOG_INFO("connection closed, connection name: {}, {}:{}", name, addr.getIP(), addr.getPort());

        std::unique_lock lock(smutx_);
        connections_.erase(name);
    });
}

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
    acceptor_(std::make_unique<Acceptor>(loop, addr)),
    threadPool_(std::make_unique<ThreadPool>(loop)) {
    acceptor_->setNewConnectionFunc([this](int fd, const Address& addr) {
        onNewConnection(fd, addr);
    });
}

detail::TcpServerImpl::~TcpServerImpl() {
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

void detail::TcpServerImpl::setConnectionCallback(ConnectionCallback cb) {
    connectionCallback_ = std::move(cb);
}

void detail::TcpServerImpl::setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
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
    connection->setCloseCallback([this](TcpConnection* conn) { onConnectionClose(conn); });
    connections_[connName] = connection;

    loop->runInLoop([&] {
        connection->connectEstablished();
    });
}

void detail::TcpServerImpl::onConnectionClose(TcpConnection* connection) {
    connection->getLoop()->runInLoop([connection] {
        connection->connectDestroyed();
    });

    loop_->runInLoop([this, connection] {
        std::string name = connection->getName();
        LOG_INFO("connection closed, connection name: {}", name);
        connections_.erase(name);
    });
}

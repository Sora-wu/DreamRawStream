//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/tcpServer.h>
#include <DreamNet/eventLoop.h>
#include <tcp/tcpServerImpl.h>
#include <tcp/eventLoopImpl.h>
#include <tcp/channel.h>

using namespace Dream;

TcpServer::TcpServer(EventLoop* loop, const Address& address) :
    impl_(new detail::TcpServerImpl(loop, address)) {
}

TcpServer::~TcpServer() {
    if (impl_) {
        delete impl_;
        impl_ = nullptr;
    }
}

void TcpServer::setThreadCount(uint32_t threadCount) const {
    impl_->setThreadNum(threadCount);
}

void TcpServer::start() const {
    impl_->start();
}

void TcpServer::setConnectionCallback(ConnectionCallback cb) const {
    impl_->setConnectionCallback(std::move(cb));
}

void TcpServer::setMessageCallback(MessageCallback cb) const {
    impl_->setMessageCallback(std::move(cb));
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/tcpConnection.h>
#include <tcp/tcpConnectionImpl.h>
#include <tcp/eventLoopImpl.h>

using namespace Dream;

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int fd, const Address& localAddress, const Address& remoteAddress) :
    impl_(std::make_unique<detail::TcpConnectionImpl>(this, loop, name, fd, localAddress, remoteAddress)),
    loop_(loop) {}

TcpConnection::~TcpConnection() = default;

EventLoop* TcpConnection::getLoop() const {
    return loop_;
}

std::string TcpConnection::getName() const {
    return impl_->getName();
}

void TcpConnection::connectEstablished() const {
    impl_->connectEstablished();
}

void TcpConnection::connectDestroyed() const {
    impl_->connectDestroyed();
}

bool TcpConnection::isConnected() const {
    return impl_->isConnected();
}

void TcpConnection::send(Buffer& buffer) const {
    impl_->send(buffer);
}

void TcpConnection::shutdown() const {
    impl_->shutdown();
}

void TcpConnection::setConnectionCallback(ConnectionCallback cb) const {
    impl_->setConnectionCallback(std::move(cb));
}

void TcpConnection::setMessageCallback(MessageCallback cb) const {
    impl_->setMessageCallback(std::move(cb));
}

void TcpConnection::setCloseCallback(CloseCallback cb) const {
    impl_->setCloseCallback(std::move(cb));
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/tcpConnection.h>
#include <tcp/tcpConnectionImpl.h>
#include <tcp/eventLoopImpl.h>

using namespace Dream;

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int fd, const Address& localAddress, const Address& remoteAddress) :
    impl_(std::make_unique<detail::TcpConnectionImpl>(this, detail::EventLoopImpl::from(loop), name, fd, localAddress, remoteAddress)),
    loop_(loop) {}

TcpConnection::~TcpConnection() = default;

EventLoop* TcpConnection::getLoop() const {
    return loop_;
}

Address TcpConnection::getLocalAddress() const {
    return impl_->getLocalAddress();
}

Address TcpConnection::getRemoteAddress() const {
    return impl_->getRemoteAddress();
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

void TcpConnection::send(const Buffer& buffer) const {
    impl_->send(buffer, shared_from_this());
}

void TcpConnection::send(const std::span<char>& buffer) const {
    impl_->send(buffer, shared_from_this());
}

void TcpConnection::send(const char* data, size_t size) const {
    impl_->send(data, size, shared_from_this());
}
void TcpConnection::shutdown() const {
    impl_->shutdown(shared_from_this());
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

void TcpConnection::setHighWaterMarkCallback(HighWaterMarkCallback cb) const {
    impl_->setHighWaterMarkCallback(std::move(cb));
}

void TcpConnection::setWriteCompleteCallback(WriteCompleteCallback cb) const {
    impl_->setWriteCompleteCallback(std::move(cb));
}

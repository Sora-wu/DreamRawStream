//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/tcpClient.h>
#include <tcp/tcpClientImpl.h>

using namespace Dream;

TcpClient::TcpClient(EventLoop* loop, const Address& address) :
    impl_(new detail::TcpClientImpl(loop, address)) {}

void TcpClient::connect() const {
    impl_->connect();
}

void TcpClient::disconnect() const {
    impl_->disconnect();
}

void TcpClient::setRetryInterval(uint32_t retryInterval) const {
    impl_->setRetryInterval(retryInterval);
}

void TcpClient::send(const Buffer& buffer) const {
    impl_->send(buffer);
}

void TcpClient::send(const std::span<char>& buffer) const {
    impl_->send(buffer);
}

void TcpClient::send(const char* data, uint32_t size) const {
    impl_->send(data, size);
}

void TcpClient::setConnectionCallback(ConnectionCallback cb) const {
    impl_->setConnectionCallback(cb);
}

void TcpClient::setMessageCallback(MessageCallback cb) const {
    impl_->setMessageCallback(cb);
}

void TcpClient::setWriteCompleteCallback(WriteCompleteCallback cb) const {
    impl_->setWriteCompleteCallback(cb);
}

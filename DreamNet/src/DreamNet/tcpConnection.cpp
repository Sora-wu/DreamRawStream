//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/tcpConnection.h>
#include <tcp/channel.h>

using namespace Dream;

TcpConnection::TcpConnection(EventLoop* eventLoop, int fd, const Address& localAddress, const Address& remoteAddress) :
    loop_(eventLoop), localAddress_(localAddress), remoteAddress_(remoteAddress) {
    socket_.setFd(fd);
}

void TcpConnection::setConnectionCallback(ConnectionCallback connectionCallback) {
    connectionCallback_ = std::move(connectionCallback);
}

void TcpConnection::setMessageCallback(MessageCallback messageCallback) {
    messageCallback_ = std::move(messageCallback);
}

void TcpConnection::setCloseCallback(CloseCallback closeCallback) {
    closeCallback_ = std::move(closeCallback);
}

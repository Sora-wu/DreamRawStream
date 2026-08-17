//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/acceptor.h>
#include <tcp/channel.h>
#include <tcp/eventLoopImpl.h>
#include <common/define.h>

#include <unistd.h>

using namespace Dream::detail;

Acceptor::Acceptor(EventLoop* loop, const Address& address) :
    loop_(loop),
    socket_(SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK) {
    acceptChannel_ = std::make_unique<Channel>(loop, socket_.getFd());
    if (!acceptChannel_) {
        LOG_FATAL("accept channel is null");
    }
    acceptChannel_->setOnReadEvent([this] { onAccept(); }); // 如果accpet有可读事件了，那么说明有新客户端连接

    initSocket(address);
}

void Acceptor::listen() {
    socket_.listen();
    acceptChannel_->enableReading();
}

void Acceptor::setNewConnectionFunc(NewConnectionFunc func) {
    newConnectionFunc_ = std::move(func);
}

void Acceptor::initSocket(const Address& address) {
    socket_.setTcpNoDelay(true);
    socket_.setReUseAddr(true);
    socket_.setReUsePort(true);
    socket_.setKeepAlive(true);
    socket_.bind(address);
}

void Acceptor::onAccept() {
    Address addr{};
    int connfd = socket_.accept(addr);
    if (connfd < 0) {
        return;
    }

    if (newConnectionFunc_) {
        newConnectionFunc_(connfd, addr);
        return;
    }

    LOG_WARN("the accept callback is not set yet, will close new client by default");
    ::close(connfd); // 如果没有注册新客户回调，那么就关闭新客户端的连接
}

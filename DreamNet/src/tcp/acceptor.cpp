//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/acceptor.h>
#include <common/define.h>

Acceptor::Acceptor(EventLoop* loop, const Dream::Address& address) : loop_(loop), socket_(SOCK_STREAM) {
    initSocket(address);

}

void Acceptor::initSocket(const Dream::Address& address) {
    socket_.setTcpNoDelay(true);
    socket_.setReUseAddr(true);
    socket_.setReUsePort(true);
    socket_.setKeepAlive(true);
    socket_.bind(address);
    socket_.listen();
}

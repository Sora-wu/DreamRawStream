//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <common/socket.h>
#include <DreamNet/address.h>
#include <DreamNet/eventLoop.h>

#include <memory>

namespace Dream {
    class EventLoop;
}

class Channel;

class Acceptor {
public:
    using NewConnectionFunc = std::function<void(int fd, const Dream::Address& addr)>;
    Acceptor(Dream::EventLoop::Impl* loop, const Dream::Address& address);

    void listen();
    void setNewConnectionFunc(NewConnectionFunc func);

private:
    void initSocket(const Dream::Address& address);
    void onAccept();

private:
    Dream::EventLoop::Impl* loop_ = nullptr;
    Socket socket_;

    std::unique_ptr<Channel> acceptChannel_;
    NewConnectionFunc newConnectionFunc_{};
};

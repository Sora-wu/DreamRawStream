//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <common/socket.h>
#include <DreamNet/address.h>

class EventLoop;
class Channel;

class Acceptor {
public:
    Acceptor(EventLoop* loop, const Dream::Address& address);

private:
    void initSocket(const Dream::Address& address);

private:
    EventLoop* loop_ = nullptr;
    Socket socket_;
};

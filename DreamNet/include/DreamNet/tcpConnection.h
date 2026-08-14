//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/buffer.hpp>
#include <DreamNet/callbacks.h>
#include <DreamNet/address.h>
#include <common/socket.h>

class Channel;

namespace Dream {
    class EventLoop;

    class TcpConnection {
    public:
        TcpConnection(EventLoop* eventLoop, int fd, const Address& localAddress, const Address& remoteAddress);

        void setConnectionCallback(ConnectionCallback connectionCallback);
        void setMessageCallback(MessageCallback messageCallback);
        void setCloseCallback(CloseCallback closeCallback);

    private:
        enum class ConnectionState {
            Disconnected,
            Connecting,
            Connected,
            Disconnecting,
        } state_ = ConnectionState::Disconnected;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        CloseCallback closeCallback_;

        EventLoop* loop_;
        Socket socket_;
        const Address& localAddress_;
        const Address& remoteAddress_;
        Channel* channel_;
        Buffer inBuffer_;
        Buffer outBuffer_;
    };
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <common/socket.h>

#include <memory>

namespace Dream::detail {
    class EventLoopImpl;
    class Channel;

    class Connector {
    public:
        using NewConnectionCallback = std::function<void(int fd, const Address& address)>;
        Connector(EventLoopImpl* loop, const Address& addr);
        void start();
        void stop() const;
        void setRetryInterval(uint32_t retryIntervalMillis);

        void setNewConnectionCallback(NewConnectionCallback newConnectionCallback);

    private:
        void handleWrite();

    private:
        EventLoopImpl* loop_ = nullptr;
        const Address& addr_;
        Socket socket_;
        std::unique_ptr<Channel> channel_;
        NewConnectionCallback newConnectionCallback_ = nullptr;
        uint32_t retryIntervalMillis_ = 100;

        enum class State {
            DISCONNECTED,
            CONNECTING,
            CONNECTED,
        } state_ = State::DISCONNECTED;
    };
}

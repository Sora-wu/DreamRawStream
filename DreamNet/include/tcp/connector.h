//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <common/socket.h>

#include <functional>
#include <memory>

namespace Dream::detail {
    class EventLoopImpl;
    class Channel;

    class Connector {
    public:
        using NewConnectionCallback = std::function<void(int fd, const Address& address)>;
        Connector(EventLoopImpl* loop, const Address& addr);
        void start();
        void stop();
        void setRetryInterval(uint32_t retryIntervalMillis);

        void setNewConnectionCallback(NewConnectionCallback newConnectionCallback);

    private:
        void initSocket() const;
        void handleWrite();
        void retry();

    private:
        EventLoopImpl* loop_ = nullptr;
        Address addr_;
        Socket socket_;
        std::unique_ptr<Channel> channel_;
        NewConnectionCallback newConnectionCallback_ = nullptr;
        uint32_t retryIntervalMillis_ = 100;
        uint32_t attemptToConnect_ = 0;

        enum class State {
            DISCONNECTED,
            CONNECTING,
            CONNECTED,
        } state_ = State::DISCONNECTED;
    };
}

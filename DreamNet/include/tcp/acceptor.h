//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <common/socket.h>
#include <DreamNet/address.h>

#include <functional>
#include <memory>

namespace Dream::detail {
    class EventLoopImpl;
    class Channel;

    class Acceptor {
    public:
        using NewConnectionFunc = std::function<void(int fd, const Dream::Address& addr)>;
        Acceptor(EventLoopImpl* loop, const Dream::Address& address);

        void listen();
        void setNewConnectionFunc(NewConnectionFunc func);

    private:
        void initSocket(const Dream::Address& address);
        void onAccept();

    private:
        EventLoopImpl* loop_ = nullptr;
        Socket socket_;

        std::unique_ptr<Channel> acceptChannel_;
        NewConnectionFunc newConnectionFunc_{};
    };
} // namespace Dream::detail

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/callbacks.h>

#include <memory>

namespace Dream {
    class EventLoop;
    class Address;

    namespace detail {
        class TcpConnectionImpl;
    }

    class TcpConnection {
    public:
        TcpConnection(EventLoop* loop, const std::string& name, int fd, const Address& localAddress, const Address& remoteAddress);
        ~TcpConnection();
        TcpConnection(const TcpConnection&) = delete;
        TcpConnection& operator=(const TcpConnection&) = delete;

        [[nodiscard]] EventLoop* getLoop() const;
        [[nodiscard]] Address getLocalAddress() const;
        [[nodiscard]] Address getRemoteAddress() const;

        [[nodiscard]] std::string getName() const;
        void connectEstablished() const;
        void connectDestroyed() const;
        [[nodiscard]] bool isConnected() const;
        void send(Buffer& buffer) const;
        void shutdown() const;

        void setConnectionCallback(ConnectionCallback cb) const;
        void setMessageCallback(MessageCallback cb) const;
        void setCloseCallback(CloseCallback cb) const;

    private:
        std::unique_ptr<detail::TcpConnectionImpl> impl_;
        EventLoop* loop_ = nullptr;
    };
}

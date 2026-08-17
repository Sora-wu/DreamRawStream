//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <tcp/channel.h>
#include <common/socket.h>
#include <DreamNet/callbacks.h>
#include <DreamNet/address.h>

#include <memory>

namespace Dream::detail {
    class EventLoopImpl;

    class TcpConnectionImpl {
        enum class ConnectionState {
            DISCONNECTED,
            CONNECTED,
        };

    public:
        TcpConnectionImpl(TcpConnection* conn, EventLoop* loop, const std::string& name, int fd,
                            const Address& localAddress, const Address& remoteAddress);
        ~TcpConnectionImpl() = default;

        [[nodiscard]] std::string getName() const;
        void connectEstablished();
        void connectDestroyed();
        [[nodiscard]] bool isConnected() const;
        void send(Buffer& buffer);
        void shutdown();

        void setConnectionCallback(ConnectionCallback cb);
        void setMessageCallback(MessageCallback cb);
        void setCloseCallback(CloseCallback cb);

    private:
        void sendInLoop(Buffer& buffer);

        void handleRead();
        void handleWrite();
        void handleClose();
        void handleError() const;

    private:
        // 非拥有
        TcpConnection* conn_ = nullptr;
        const std::string name_{};

        ConnectionState state_ = ConnectionState::DISCONNECTED;

        EventLoop* loop_ = nullptr;
        Socket socket_;
        Address localAddress_;
        Address remoteAddress_;
        std::unique_ptr<Channel> channel_;
        Buffer inBuffer_;
        Buffer outBuffer_;

        ConnectionCallback connectionCallback_ = nullptr;
        MessageCallback messageCallback_ = nullptr;
        CloseCallback closeCallback_ = nullptr;
    };
}

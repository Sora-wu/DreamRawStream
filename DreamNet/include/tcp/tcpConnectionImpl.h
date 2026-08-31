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
            DISCONNECTING,
            DISCONNECTED,
            CONNECTING,
            CONNECTED,
        };

    public:
        TcpConnectionImpl(TcpConnection* conn, EventLoopImpl* loop, const std::string& name, int fd,
                            const Address& localAddress, const Address& remoteAddress);

        [[nodiscard]] Address getLocalAddress() const;
        [[nodiscard]] Address getRemoteAddress() const;

        [[nodiscard]] std::string getName() const;
        void connectEstablished();
        void connectDestroyed();
        [[nodiscard]] bool isConnected() const;
        void send(const Buffer& buffer, std::shared_ptr<const TcpConnection> self);
        void send(const std::span<char>& buffer, std::shared_ptr<const TcpConnection> self);
        void send(const char* data, size_t size, std::shared_ptr<const TcpConnection> self);
        void shutdown(std::shared_ptr<const TcpConnection> self);
        void close(std::shared_ptr<const TcpConnection> self);

        void setConnectionCallback(ConnectionCallback cb);
        void setMessageCallback(MessageCallback cb);
        void setCloseCallback(CloseCallback cb);
        void setHighWaterMarkCallback(HighWaterMarkCallback cb);
        void setWriteCompleteCallback(WriteCompleteCallback cb);

    private:
        void sendInLoop(Buffer& buffer);

        void handleRead();
        void handleWrite();
        void handleClose();
        void handleError();

    private:
        // 非拥有
        TcpConnection* conn_ = nullptr;
        const std::string name_{};

        ConnectionState state_ = ConnectionState::DISCONNECTED;

        EventLoopImpl* loop_ = nullptr;
        Socket socket_;
        Address localAddress_;
        Address remoteAddress_;
        std::unique_ptr<Channel> channel_;
        Buffer inBuffer_;
        Buffer outBuffer_;
        bool shutdownAfterWrite_ = false;

        ConnectionCallback connectionCallback_ = nullptr;
        MessageCallback messageCallback_ = nullptr;
        CloseCallback closeCallback_ = nullptr;
        HighWaterMarkCallback highWaterMarkCallback_ = nullptr;
        WriteCompleteCallback writeCompleteCallback_ = nullptr;
    };
}

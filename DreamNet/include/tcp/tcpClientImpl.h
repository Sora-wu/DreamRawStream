//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/address.h>
#include <DreamNet/callbacks.h>

#include <memory>

namespace Dream {
    class EventLoop;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    namespace detail {
        class Connector;

        class TcpClientImpl {
        public:
            TcpClientImpl(EventLoop* loop, const Address& addr);

            void connect() const;
            void disconnect() const;
            void setRetryInterval(uint32_t retryInterval) const;
            void send(const Buffer& buffer) const;
            void send(const std::span<char>& buffer) const;
            void send(const char* data, uint32_t size) const;

            void setConnectionCallback(ConnectionCallback cb);
            void setMessageCallback(MessageCallback cb);
            void setWriteCompleteCallback(WriteCompleteCallback cb);

        private:
            void onNewConnection(int fd, const Address& peerAddr);
            void onConnectionClose(TcpConnection* connection) const;

        private:
            EventLoop* loop_ = nullptr;
            std::unique_ptr<Connector> connector_;
            TcpConnectionPtr connection_;

            ConnectionCallback connectionCallback_ = nullptr;
            MessageCallback messageCallback_ = nullptr;
            WriteCompleteCallback writeCompleteCallback_ = nullptr;
        };
    }
}

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

    namespace detail {
        class Connector;

        class TcpClientImpl {
        public:
            TcpClientImpl(EventLoop* loop, const Address& addr);

            void connect();
            void disconnect();
            void setRetryInterval(uint32_t retryInterval) const;
            void send(const Buffer& buffer) const;
            void send(const std::span<char>& data) const;
            void send(const char* data, uint32_t size) const;

            void setConnectionCallback(ConnectionCallback cb);
            void setMessageCallback(MessageCallback cb);
            void setWriteCompleteCallback(WriteCompleteCallback cb);
            void setHighWaterMarkCallback(HighWaterMarkCallback cb);

        private:
            void onNewConnection(int fd, const Address& peerAddr);
            void onConnectionClose(TcpConnectionPtr connection);

        private:
            EventLoop* loop_ = nullptr;
            std::unique_ptr<Connector> connector_;
            TcpConnectionPtr connection_;
            bool manualDisconnect_ = false;

            ConnectionCallback connectionCallback_ = nullptr;
            MessageCallback messageCallback_ = nullptr;
            WriteCompleteCallback writeCompleteCallback_ = nullptr;
            HighWaterMarkCallback highWaterMarkCallback_ = nullptr;
        };
    }
}

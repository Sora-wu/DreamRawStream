//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/address.h>
#include <DreamNet/callbacks.h>

namespace Dream {
    class EventLoop;

    namespace detail {
        class TcpServerImpl;
    }

    class TcpServer {
    public:
        TcpServer(EventLoop* loop, const Address& address);
        ~TcpServer();
        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;

        void setThreadCount(uint32_t threadCount) const;
        void start() const;
        // void forEachConnect(std::function<void(TcpConnection*)> callback) const;
        // uint32_t getConnectionCount() const;
        void sendBroadcast(const Buffer& buffer) const;
        void sendBroadcast(const std::span<char>& buffer) const;
        void sendBroadcast(const char* data, uint32_t size) const;

        void setConnectionCallback(ConnectionCallback cb) const;
        void setMessageCallback(MessageCallback cb) const;
        void setHighWaterMarkCallback(HighWaterMarkCallback cb) const;
        void setWriteCompleteCallback(WriteCompleteCallback cb) const;

    private:
        detail::TcpServerImpl* impl_ = nullptr;
    };
}

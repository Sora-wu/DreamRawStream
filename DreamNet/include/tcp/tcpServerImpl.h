//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/callbacks.h>
#include <DreamNet/address.h>

#include <memory>
#include <atomic>
#include <string>
#include <unordered_map>

namespace Dream {
    class TcpServer;
    class EventLoop;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    namespace detail {
        class Acceptor;
        class ThreadPool;

        class TcpServerImpl {
        public:
            TcpServerImpl(EventLoop* loop, const Address& addr);
            ~TcpServerImpl();

            void setThreadNum(uint32_t threadCount) const;
            void start();

            void setConnectionCallback(ConnectionCallback cb);
            void setMessageCallback(MessageCallback cb);

        private:
            void onNewConnection(int fd, const Address& addr);
            void onConnectionClose(TcpConnection* connection);

        private:
            EventLoop* loop_ = nullptr;
            std::unique_ptr<Acceptor> acceptor_;
            std::unique_ptr<ThreadPool> threadPool_;

            ConnectionCallback connectionCallback_ = nullptr;
            MessageCallback messageCallback_ = nullptr;

            std::atomic_bool isRunning_ = false;
            std::unordered_map<std::string, TcpConnectionPtr> connections_;
            uint32_t connectionID = 0;
        };
    }
}

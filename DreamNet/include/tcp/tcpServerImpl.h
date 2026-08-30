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
#include <shared_mutex>

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

            void forEachConnect(const std::function<void(TcpConnection*)>& callback);
            uint32_t getConnectionCount();
            void sendBroadcast(const Buffer& buffer);
            void sendBroadcast(const std::span<char>& buffer);
            void sendBroadcast(const char* data, uint32_t size);

            void setConnectionCallback(ConnectionCallback cb);
            void setMessageCallback(MessageCallback cb);
            void setHighWaterMarkCallback(HighWaterMarkCallback cb);
            void setWriteCompleteCallback(WriteCompleteCallback cb);

        private:
            void onNewConnection(int fd, const Address& addr);
            void onConnectionClose(TcpConnectionPtr connection);

        private:
            EventLoop* loop_ = nullptr;
            std::unique_ptr<Acceptor> acceptor_;
            std::unique_ptr<ThreadPool> threadPool_;

            ConnectionCallback connectionCallback_ = nullptr;
            MessageCallback messageCallback_ = nullptr;
            // 一个慢客户端能吃几个GB，把整个进程拖死，健康客户端也一起挂
            // 慢客户端的缓冲里堆了几分钟前的旧画面，它好不容易追上时，看到的是很久以前的内容——直播里这比断线更糟。
            HighWaterMarkCallback highWaterMarkCallback_ = nullptr;
            WriteCompleteCallback writeCompleteCallback_ = nullptr;

            std::atomic_bool isRunning_ = false;
            std::unordered_map<std::string, TcpConnectionPtr> connections_;
            uint32_t connectionID = 0;
            std::shared_mutex smutx_{};
        };
    }
}

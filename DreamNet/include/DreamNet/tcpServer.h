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
        class TcpServerImpl;
    }

    class TcpServer {
    public:
        TcpServer(EventLoop* loop, const Address& address);
        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;

        void setThreadCount(uint32_t threadCount) const;
        void start() const;

        void setConnectionCallback(ConnectionCallback cb) const;
        void setMessageCallback(MessageCallback cb) const;

    private:
        std::unique_ptr<detail::TcpServerImpl> impl_;
    };
}

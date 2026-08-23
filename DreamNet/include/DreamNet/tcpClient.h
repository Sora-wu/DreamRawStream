//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/callbacks.h>
#include <DreamNet/address.h>

namespace Dream {
    class EventLoop;

    namespace detail {
        class TcpClientImpl;
    }

    class TcpClient {
    public:
        TcpClient(EventLoop* loop, const Address& address);
        ~TcpClient();

        void connect() const;
        void disconnect() const;
        void setRetryInterval(uint32_t retryInterval) const;
        void send(const Buffer& buffer) const;
        void send(const std::span<char>& buffer) const;
        void send(const char* data, uint32_t size) const;

        void setConnectionCallback(ConnectionCallback cb) const;
        void setMessageCallback(MessageCallback cb) const;
        void setWriteCompleteCallback(WriteCompleteCallback cb) const;

    private:
        detail::TcpClientImpl* impl_ = nullptr;
    };
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/buffer.hpp>

#include <functional>
#include <cstdint>
#include <memory>

namespace Dream {
    class TcpConnection;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(TcpConnectionPtr)>;
    using MessageCallback = std::function<uint32_t(TcpConnectionPtr, Buffer&)>;
    using CloseCallback = std::function<void(TcpConnectionPtr)>;
    using HighWaterMarkCallback = std::function<void(TcpConnectionPtr)>;
    using WriteCompleteCallback = std::function<void(TcpConnectionPtr)>;

    using Functor = std::function<void()>;
}
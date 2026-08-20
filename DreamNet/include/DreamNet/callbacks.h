//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/buffer.hpp>

#include <functional>

namespace Dream {
    class TcpConnection;
    using ConnectionCallback = std::function<void(TcpConnection*)>;
    using MessageCallback = std::function<void(TcpConnection*, Buffer&)>;
    using CloseCallback = std::function<void(TcpConnection*)>;
    using HighWaterMarkCallback = std::function<void(TcpConnection*)>;
    using WriteCompleteCallback = std::function<void(TcpConnection*)>;
}
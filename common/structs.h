//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <cstdint>
#include <memoryPool.hpp>

enum class FrameType {
    VIDEO,
    AUDIO,
};

struct Frame {
    FrameType type;
    PooledBuffer buffer;
    int64_t pts;
};

struct FrameHeader {
    int64_t pts = 0;
    uint32_t size = 0;
    uint8_t type = 0;           // 0=Video 1=Audio
};
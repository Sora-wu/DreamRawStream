//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <cstdint>
#include <memoryPool.hpp>

enum class FrameType {
    Video,
    Audio,
};

struct Frame {
    FrameType type;
    PooledBuffer buffer;
    int64_t pts;
};

#pragma pack(push, 1)
struct FrameHeader {
    uint8_t type = 0;           // 0=Video 1=Audio
    uint32_t size = 0;
    int64_t pts = 0;
};
#pragma pack(pop)
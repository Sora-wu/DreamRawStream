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


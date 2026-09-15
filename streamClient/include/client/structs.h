//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <structs.h>

struct DecodeFrame {
    uint32_t streamID;
    Frame frame;
};

// UI层视频数据，与ffmpeg细节分离
struct VideoFrame {
    uint32_t frameWidth{};
    uint32_t frameHeight{};
    double picWidthHeightRatio{};
    int64_t pts{};

    char* data[3]{};
    int len[3]{};
    int stride[3]{};
};

// UI层视频数据，与ffmpeg细节分离
struct AudioFrame {
    char* data{};
    int len{};
    uint32_t capacity = 0;
    int64_t pts{};
};
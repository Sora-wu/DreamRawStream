//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <cstring>
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

    VideoFrame& operator=( const VideoFrame& o) {
        frameWidth = o.frameWidth;
        frameHeight = o.frameHeight;
        picWidthHeightRatio = o.picWidthHeightRatio;
        pts = o.pts;

        for (uint32_t i = 0; i < 3; ++i) {
            if (!o.data[i] || o.len[i] == 0) {
                continue;
            }

            if (len[i] < o.len[i]) {
                delete[] data[i];
                len[i] = o.len[i];
                data[i] = new char[len[i]];
            }

            memcpy(data[i], o.data[i], len[i]);
            stride[i] = o.stride[i];
        }

        return *this;
    }
};

// UI层视频数据，与ffmpeg细节分离
struct AudioFrame {
    char* data{};
    int len{};
    uint32_t capacity = 0;
    int64_t pts{};
};
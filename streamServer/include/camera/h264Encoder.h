//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <cstdint>
#include <span>

#include <x264.h>

class H264Encoder {
public:
    H264Encoder(uint32_t width, uint32_t height);
    ~H264Encoder();

    [[nodiscard]] std::span<char> encode(const char* inBuffer);

private:
    x264_t* handler_{};
    x264_param_t param_{};
    x264_picture_t pic_{};
    x264_nal_t* nal_{};

    int64_t pts_ = 0;
    char* encodeBuffer_ = nullptr;
    uint32_t encodeBufferCapacity_ = 0;
};

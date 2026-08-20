//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include <fdk-aac/aacenc_lib.h>

class FdkAACEncoder {
public:
    FdkAACEncoder(uint32_t sampleRate, uint32_t channels, uint32_t bytesPerSample);
    ~FdkAACEncoder();

    FdkAACEncoder(const FdkAACEncoder&) = delete;
    FdkAACEncoder& operator=(const FdkAACEncoder&) = delete;

    [[nodiscard]] std::span<char> encode(std::span<char> inPcmBuffer);

private:
    void init();

private:
    uint32_t sampleRate_;
    uint32_t channels_;
    uint32_t bytesPerSample_;

    HANDLE_AACENCODER aacHandle_ = nullptr;
    std::vector<char> outBuffer_;
};

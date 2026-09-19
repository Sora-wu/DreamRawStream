//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <FFmpegPool.hpp>
#include <client/structs.h>

#include <functional>
#include <memory>

extern "C" {
struct AVCodecContext;
struct SwsContext;
struct AVFrame;
}

class VideoDecoder {
public:
    using OnVideoFrameFunc = std::function<void(const VideoFrame&)>;

    VideoDecoder();
    ~VideoDecoder();

    void decode(const char* data, uint32_t size, int64_t pts, OnVideoFrameFunc func);

private:
    void refresh(AVFrame* frame);

private:
    AVCodecContext* codecCtx_ = nullptr;
    SwsContext* swsContext_ = nullptr;
    int cacheWidth_ = 0;
    int cacheHeight_ = 0;
    AVPixelFormat cacheFormat_ = AV_PIX_FMT_NONE;
    VideoFrame videoFrame_{};
    bool isStarted_ = false;

    std::unique_ptr<AVFramePool> framePool_;
    std::unique_ptr<AVPacketPool> packetPool_;
};

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <client/structs.h>
#include <FFmpegPool.hpp>

#include <functional>
#include <memory>

extern "C" {
struct AVCodec;
struct AVCodecContext;
struct AVCodecParserContext;
struct AVFrame;
struct SwrContext;
}

class AudioDecoder {
public:
    using OnAudioFrameFunc = std::function<void(const AudioFrame&)>;

    AudioDecoder();
    ~AudioDecoder();

    void decode(const char* data, uint32_t size, int64_t pts, OnAudioFrameFunc func);

private:
    void refresh(AVFrame* frame);
    bool convertFrame(AVFrame* before, AVFrame* after) const;

private:
    const AVCodec* codec_ = nullptr;
    AVCodecContext* codecCtx_ = nullptr;
    AVCodecParserContext* parserCtx_ = nullptr;
    AudioFrame audioFrame_{};

    SwrContext* swrCtx_ = nullptr;
    uint64_t cachedChannelLayout_ = 0;
    int cachedFormat_ = AV_SAMPLE_FMT_NONE;
    int cachedSampleRate_ = 0;

    std::unique_ptr<AVPacketPool> packetPool_;
    std::unique_ptr<AVFramePool> framePool_;

    bool isOpened_ = false;
};

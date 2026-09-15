//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <vector>

extern "C" {
#include <libavutil/frame.h>
#include <libavcodec/packet.h>
}

template <typename T>
struct FFmpegTraits;

template <>
struct FFmpegTraits<AVFrame> {
    static AVFrame* alloc() {
        return av_frame_alloc();
    }

    static void free(AVFrame** frame) {
        av_frame_free(frame);
    }

    static void unref(AVFrame* frame) {
        av_frame_unref(frame);
    }
};

template <>
struct FFmpegTraits<AVPacket> {
    static AVPacket* alloc() {
        return av_packet_alloc();
    }

    static void free(AVPacket** packet) {
        av_packet_free(packet);
    }

    static void unref(AVPacket* packet) {
        av_packet_unref(packet);
    }
};

template <typename T>
class FFmpegPool {
public:
    ~FFmpegPool() {
        clear();
    }

    [[nodiscard]] T* get() {
        if (!pool_.empty()) {
            T* frame = pool_.back();
            pool_.pop_back();

            // 给出去的时候，unref一下，避免上次使用的脏数据
            FFmpegTraits<T>::unref(frame);
            return frame;
        }

        return FFmpegTraits<T>::alloc();
    }

    // remainBuffer为false一般是在avcodec_receive_*时使用，因为内存来自编码器/解码器，需要归还
    void put(T* object, bool remainBuffer = true) {
        if (!object) {
            return;
        }

        if (!remainBuffer) {
            FFmpegTraits<T>::unref(object);
        }
        pool_.push_back(object);
    }

    void clear() {
        for (T* frame : pool_) {
            FFmpegTraits<T>::free(&frame);
        }

        pool_.clear();
    }

private:
    std::vector<T*> pool_;
};

using AVFramePool = FFmpegPool<AVFrame>;
using AVPacketPool = FFmpegPool<AVPacket>;
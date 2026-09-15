//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <decoder/videoDecoder.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace {
    constexpr AVPixelFormat PIXEL_FORMAT = AV_PIX_FMT_YUV420P;

    void updateVideoFrameSize(VideoFrame* videoFrame, int height, const int linesize[4]) {
        const int planeHeights[3] = { height, height / 2, height / 2 };
        for (uint32_t i = 0; i < 3; ++i) {
            if (!linesize[i]) {
                continue;
            }
            const int bufSize = linesize[i] * planeHeights[i];
            if (bufSize == videoFrame->len[i] && videoFrame->data[i]) {
                continue;
            }
            if (videoFrame->data[i]) {
                delete[] videoFrame->data[i];
                videoFrame->data[i] = nullptr;
            }
            videoFrame->data[i] = new char[bufSize]{};
            videoFrame->len[i] = bufSize;
            videoFrame->stride[i] = linesize[i];
        }
    }
}

VideoDecoder::VideoDecoder() {
    framePool_ = std::make_unique<AVFramePool>();
    packetPool_ = std::make_unique<AVPacketPool>();

    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    codecCtx_ = avcodec_alloc_context3(codec);
    avcodec_open2(codecCtx_, codec, nullptr);
}

VideoDecoder::~VideoDecoder() {
    if (swsContext_) {
        sws_freeContext(swsContext_);
        swsContext_ = nullptr;
    }
}

void VideoDecoder::decode(const char* data, uint32_t size, int64_t pts, OnVideoFrameFunc func) {
    AVPacket* pkt = packetPool_->get();
    pkt->data = (uint8_t*)data;
    pkt->size = size;
    pkt->pts = pts;
    pkt->dts = AV_NOPTS_VALUE;

    if (avcodec_send_packet(codecCtx_, pkt) < 0) {
        packetPool_->put(pkt);
        return;
    }

    AVFrame* frame = framePool_->get();
    while (avcodec_receive_frame(codecCtx_, frame) == 0) {
        refresh(frame);
        sws_scale(swsContext_, frame->data, frame->linesize,
            0, frame->height,
            (uint8_t* const*)videoFrame_.data, videoFrame_.stride);
        videoFrame_.pts = pts;
        func(videoFrame_);

        // 在循环使用时，先释放之前的
        av_frame_unref(frame);
    }
    framePool_->put(frame);
    packetPool_->put(pkt);
}

void VideoDecoder::refresh(AVFrame* frame) {
    if (frame->width != cacheWidth_ || frame->height != cacheHeight_ || frame->format != cacheFormat_) {
        swsContext_ = sws_getCachedContext(swsContext_,
            frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, PIXEL_FORMAT,
            SWS_BICUBIC, nullptr, nullptr, nullptr);
        if (!swsContext_) {
            av_log(nullptr, AV_LOG_ERROR, "swsContext_ failed\n");
            return;
        }

        cacheWidth_ = frame->width;
        cacheHeight_ = frame->height;
        cacheFormat_ = (AVPixelFormat)frame->format;
    }

    videoFrame_.frameWidth = frame->width;
    videoFrame_.frameHeight = frame->height;
    videoFrame_.picWidthHeightRatio = frame->sample_aspect_ratio.den == 0 ? 0 : av_q2d(frame->sample_aspect_ratio);
    int outLinesize[4]{};
    if (av_image_fill_linesizes(outLinesize, PIXEL_FORMAT, frame->width) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "av_image_fill_linesizes failed\n");
        return;
    }

    updateVideoFrameSize(&videoFrame_, frame->height, outLinesize);
}

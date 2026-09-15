//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <decoder/audioDecoder.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

namespace {
    constexpr int SAMPLE_RATE = 44100;
    constexpr AVSampleFormat SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;
    constexpr uint64_t CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
    constexpr uint32_t CHANNEL_COUNT = 2;

    void updateAudioFrameSize(AudioFrame* audioFrame, int buffSize) {
        memset(audioFrame->data, 0, audioFrame->capacity);

        if (audioFrame->capacity >= buffSize) {
            return;
        }

        delete[] audioFrame->data;
        audioFrame->data = new char[buffSize]{};
        audioFrame->len = buffSize;
        audioFrame->capacity = buffSize;
    }
}

AudioDecoder::AudioDecoder() {
    packetPool_ = std::make_unique<AVPacketPool>();

    codec_ = avcodec_find_decoder(AV_CODEC_ID_AAC);
    codecCtx_ = avcodec_alloc_context3(codec_);
    parserCtx_ = av_parser_init(AV_CODEC_ID_AAC);
}

AudioDecoder::~AudioDecoder() {
    if (swrCtx_) {
        swr_free(&swrCtx_);
        swrCtx_ = nullptr;
    }
}

void AudioDecoder::decode(const char* data, uint32_t size, int64_t pts, OnAudioFrameFunc func) {
    const uint8_t* in = (const uint8_t*)data;
    uint32_t inSize = size;

    while (inSize > 0) {
        uint8_t* out = nullptr;
        int outSize = 0;
        int consumed = av_parser_parse2(parserCtx_, codecCtx_, &out, &outSize, in, inSize,
            AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (consumed <= 0) {
            break;
        }

        inSize -= consumed;
        in += consumed;

        if (!isOpened_ && codecCtx_->extradata_size > 0) {
            avcodec_open2(codecCtx_, codec_, nullptr);
            isOpened_ = true;
        }
        if (!isOpened_) {
            continue;
        }

        AVPacket* pkt = packetPool_->get();
        pkt->data = out;
        pkt->size = outSize;
        pkt->pts = pts;
        pkt->dts = AV_NOPTS_VALUE;
        if (avcodec_send_packet(codecCtx_, pkt) < 0) {
            packetPool_->put(pkt);
            return;
        }

        AVFrame* frame = framePool_->get();
        while (avcodec_receive_frame(codecCtx_, frame) == 0) {
            refresh(frame);

            AVFrame* convertedFrame = framePool_->get();
            if (!convertFrame(frame, convertedFrame)) {
                framePool_->put(convertedFrame);
                break;
            }
            const int bufferSize = av_samples_get_buffer_size(nullptr, convertedFrame->channels,
                convertedFrame->nb_samples, (AVSampleFormat)convertedFrame->format, 1);
            audioFrame_.len = bufferSize;
            updateAudioFrameSize(&audioFrame_, bufferSize);
            memcpy(audioFrame_.data, convertedFrame->data[0], bufferSize);
            audioFrame_.pts = pts;
            func(audioFrame_);
            framePool_->put(convertedFrame);

            // 在循环使用时，先释放之前的
            av_frame_unref(frame);
        }
        framePool_->put(frame);
        packetPool_->put(pkt);
    }
}

void AudioDecoder::refresh(AVFrame* frame) {
    if (frame->sample_rate != cachedSampleRate_ || frame->format != cachedFormat_ || frame->channel_layout != cachedChannelLayout_) {
        swr_free(&swrCtx_);
        swrCtx_ = swr_alloc_set_opts(nullptr,
                                    CHANNEL_LAYOUT, SAMPLE_FORMAT, SAMPLE_RATE,
                                    frame->channel_layout, (AVSampleFormat)frame->format, frame->sample_rate,
                                    0, nullptr);
        if (!swrCtx_ || swr_init(swrCtx_) < 0) {
            av_log(nullptr, AV_LOG_ERROR, "swr_alloc_set_opts failed\n");
            swr_free(&swrCtx_);
            cachedChannelLayout_ = 0;
            cachedFormat_ = AV_SAMPLE_FMT_NONE;
            cachedSampleRate_ = 0;
            return;
        }
        cachedChannelLayout_ = frame->channel_layout;
        cachedFormat_ = frame->format;
        cachedSampleRate_ = frame->sample_rate;
    }
}

bool AudioDecoder::convertFrame(AVFrame* before, AVFrame* after) const {
    const int64_t delay = swr_get_delay(swrCtx_, before->sample_rate);
    const int outSampleCount = (int)av_rescale_rnd(before->nb_samples + delay, SAMPLE_RATE, before->sample_rate, AV_ROUND_UP);

    after->format = SAMPLE_FORMAT;
    after->sample_rate = SAMPLE_RATE;
    after->channel_layout = CHANNEL_LAYOUT;
    after->channels = CHANNEL_COUNT;
    after->nb_samples = outSampleCount;
    if (av_frame_get_buffer(after, 0) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "av_frame_get_buffer failed\n");
        return false;
    }
    const int convered = swr_convert(swrCtx_, after->data, outSampleCount,
                                    (const uint8_t**)before->extended_data, before->nb_samples);
    if (convered <= 0) {
        av_log(nullptr, AV_LOG_ERROR, "swr_convert failed\n");
        return false;
    }

    // 确保所需的属性正确，因为已经经过重采样
    after->format = SAMPLE_FORMAT;
    after->sample_rate = SAMPLE_RATE;
    after->channel_layout = CHANNEL_LAYOUT;
    after->channels = CHANNEL_COUNT;
    after->nb_samples = convered;

    return true;
}

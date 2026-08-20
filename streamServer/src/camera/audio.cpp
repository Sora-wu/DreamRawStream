//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <camera/audio.h>

#include <print>

namespace {
    constexpr uint32_t SAMPLE_RATE = 16000;
    constexpr uint32_t CHANNELS = 1;
    constexpr snd_pcm_uframes_t PREF_BUFFER_FRAMES = 1024;
    constexpr snd_pcm_format_t SAMPLE_FORMAT = SND_PCM_FORMAT_S16_LE;
}

Audio::Audio() {
    init();
}

Audio::~Audio() {
    if (buffer_) {
        delete[] buffer_;
        buffer_ = nullptr;
    }

    if (handle_) {
        snd_pcm_drop(handle_);
        snd_pcm_close(handle_);
        handle_ = nullptr;
    }
}

uint32_t Audio::getSampleRate() const {
    return actualSampleRate_;
}

uint32_t Audio::getChannels() const {
    return CHANNELS;
}

uint32_t Audio::getBytesPerSample() const {
    return snd_pcm_format_physical_width(SAMPLE_FORMAT) / 8;
}

AudioFrame Audio::getBuffer() {
    int res = snd_pcm_readi(handle_, buffer_, frames_);
    if (res == -EPIPE) {
        std::println(stderr, "Underrun occurred");
        snd_pcm_prepare(handle_);
        return {};
    }
    else if (res < 0) {
        std::println(stderr, "Error during read: {}", snd_strerror(res));
        return {};
    }
    else if (res != (int)frames_) {
        std::println(stderr, "Short write (expected {}, wrote {})", res, frames_);
    }

    const int64_t pts = (int64_t)(totalFramesRead_ * 1000000 / actualSampleRate_);
    totalFramesRead_ += res;
    uint32_t bytes = frameBytes_ * res;
    return { std::span(buffer_, bytes), pts };
}

void Audio::init() {
    int res = snd_pcm_open(&handle_, "default", SND_PCM_STREAM_CAPTURE, 0);
    if (res < 0) {
        std::println(stderr, "Failed to open audio device: {}", snd_strerror(res));
        exit(2);
    }

    snd_pcm_hw_params_t* params = nullptr;
    snd_pcm_hw_params_alloca(&params);          // 分配内存
    snd_pcm_hw_params_any(handle_, params);     // 设置默认参数
    snd_pcm_hw_params_set_access(handle_, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle_, params, SAMPLE_FORMAT);
    snd_pcm_hw_params_set_channels(handle_, params, CHANNELS);     // 设置声道

    uint32_t sampleRate = SAMPLE_RATE;
    int dir = 0;
    snd_pcm_hw_params_set_rate_near(handle_, params, &sampleRate, &dir);
    actualSampleRate_ = sampleRate;             // 获取与硬件协商实际的采样率
    std::println("actual sample rate: {}", sampleRate);

    frames_ = PREF_BUFFER_FRAMES;
    snd_pcm_hw_params_set_period_size_near(handle_, params, &frames_, &dir);

    res = snd_pcm_hw_params(handle_, params);
    if (res < 0) {
        snd_pcm_close(handle_);
        std::println(stderr, "Failed to set audio parameters: {}", snd_strerror(res));
        exit(2);
    }

    snd_pcm_hw_params_get_period_size(params, &frames_, &dir);
    snd_pcm_hw_params_get_period_time(params, &sampleRate, &dir);

    const uint32_t bytePerSample = snd_pcm_format_physical_width(SAMPLE_FORMAT) / 8;
    frameBytes_ = bytePerSample * CHANNELS;
    const uint32_t totalBufferSize = frameBytes_ * frames_;
    buffer_ = new char[totalBufferSize]{};
    assert(buffer_);
}

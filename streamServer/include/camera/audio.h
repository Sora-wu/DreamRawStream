//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <span>

#include <alsa/asoundlib.h>

struct AudioFrame {
    std::span<char> data;
    int64_t pts;
};

class Audio {
public:
    Audio();
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    [[nodiscard]] uint32_t getSampleRate() const;
    [[nodiscard]] uint32_t getChannels() const;
    [[nodiscard]] uint32_t getBytesPerSample() const;

    [[nodiscard]] AudioFrame getBuffer();

private:
    void init();

private:
    snd_pcm_t* handle_ = nullptr;
    snd_pcm_uframes_t frames_{};
    char* buffer_ = nullptr;

    uint32_t frameBytes_ = 0;

    uint32_t actualSampleRate_ = 0;
    uint64_t totalFramesRead_ = 0;
};

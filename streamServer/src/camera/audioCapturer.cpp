//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <camera/audioCapturer.h>
#include <structs.h>

#include <cassert>
#include <print>

void AudioCapturer::setAudio(Audio* audio) {
    audio_.reset(audio);
}

void AudioCapturer::run(std::stop_token st) {
    assert(audio_);
    const uint32_t sampleRate = audio_->getSampleRate();
    const uint32_t channels = audio_->getChannels();
    const uint32_t bytesPerSample = audio_->getBytesPerSample();
    aacEncoder_ = std::make_unique<FdkAACEncoder>(sampleRate, channels, bytesPerSample);

    while (!st.stop_requested()) {
        AudioFrame frame = audio_->getBuffer();
        if (!frame.data.empty()) {
            auto captureTime = std::chrono::steady_clock::now();
            const int64_t currentPTS = std::chrono::duration_cast<std::chrono::milliseconds>(captureTime - baseTime_).count();
            std::span<char> encodeBuffer = aacEncoder_->encode(frame.data);
            if (!encodeBuffer.empty()) {
                char* buffer = pool_.allocate(encodeBuffer.size());
                memcpy(buffer, encodeBuffer.data(), encodeBuffer.size());
                PooledBuffer pooledBuffer{ &pool_, buffer, (uint32_t)encodeBuffer.size() };
                Frame frame{ FrameType::Audio, std::move(pooledBuffer), currentPTS };
                handle(&frame);
            }
        }

        msleep(3);
    }

    std::println("CameraCapturer thread exit");
}

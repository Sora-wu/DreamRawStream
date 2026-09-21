//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <audio/audioBufferDevice.h>

namespace {
    constexpr uint32_t MAX_BUFFER_SIZE = 1024 * 64;         // 音频缓冲区最大64k
}

AudioBufferDevice::AudioBufferDevice(QObject* parent) {
    QIODevice::open(ReadWrite);
}

AudioBufferDevice::~AudioBufferDevice() {
}

void AudioBufferDevice::onAudioFrame(const AudioFrame& audioFrame) {
    std::unique_lock locker(mutex_);
    audioFrame_ = audioFrame;
    if (audioFrame_.updated) {
        cv_.wait(locker, [&] { return audioData_.size() + audioFrame_.stride <= MAX_BUFFER_SIZE; });
        QByteArray data{ audioFrame_.data, audioFrame_.stride };
        audioData_.push_back(std::move(data));
        audioFrame_.updated = false;
        cv_.notify_one();
    }
}

qint64 AudioBufferDevice::bytesAvailable() const {
    std::lock_guard lock(mutex_);
    return QIODevice::bytesAvailable() + audioData_.size();
}

qint64 AudioBufferDevice::readData(char* data, qint64 maxlen) {
    std::unique_lock locker(mutex_);
    cv_.wait(locker, [&]{ return audioData_.size() > 0; });

    const qint64 readLen = std::min(maxlen, audioData_.size());
    memcpy(data, audioData_.data(), readLen);
    audioData_.remove(0, readLen);

    cv_.notify_one();
    return readLen;
}

qint64 AudioBufferDevice::writeData(const char* data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}
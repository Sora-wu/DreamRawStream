//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <decoder/IAudioSink.h>
#include <client/structs.h>

#include <QIODevice>
#include <QByteArray>

#include <mutex>
#include <condition_variable>

class AudioBufferDevice : public QIODevice, public IAudioSink {
public:
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int CHANNEL_COUNT = 2;
    static constexpr int SAMPLE_SIZE = 2;

    explicit AudioBufferDevice(QObject *parent = nullptr);
    ~AudioBufferDevice() override;

    void onAudioFrame(const AudioFrame& audioFrame) override;

    qint64 bytesAvailable() const override;

protected:

    qint64 readData(char* data, qint64 maxlen) override;
    // 禁用 writeData，数据由外部写入
    qint64 writeData(const char *data, qint64 len) override;

private:
    AudioFrame audioFrame_{};
    QByteArray audioData_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

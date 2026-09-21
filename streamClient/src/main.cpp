//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <widget/clientWindow.h>
#include <widget/videoWidget.h>
#include <client/decodeScheduler.h>
#include <audio/audioBufferDevice.h>

#include <thread>
#include <vector>
#include <memory>

#include <QApplication>
#include <QSurfaceFormat>
#include <QAudioSink>
#include <QMediaDevices>

#include <DreamNet/DreamNet.h>

namespace {
    void setSurfaceFormat() {
        QSurfaceFormat format;
        format.setVersion(4, 5);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
        format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
        QSurfaceFormat::setDefaultFormat(format);
    }

    void setupVideoWidget(DecodeScheduler& scheduler, ClientWindow& w) {
        for (uint32_t i = 0; i < DecodeScheduler::MAX_STREAM_COUNT; ++i) {
            scheduler.setVideoSink(i, w.getVideoWidget(i));
        }
    }

    std::unique_ptr<QAudioSink> setupAudio(DecodeScheduler& scheduler, AudioBufferDevice& audioBufferDevice) {
        QAudioFormat format;
        format.setSampleRate(AudioBufferDevice::SAMPLE_RATE);
        format.setChannelCount(AudioBufferDevice::CHANNEL_COUNT);
        format.setSampleFormat(QAudioFormat::Int16);
        QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
        if (!audioDevice.isFormatSupported(format)) {
            qInfo() << "sample rate: " << format.sampleRate();
            qInfo() << "channel count: " << format.channelCount();
            qWarning() << "can not support this audio format or detect any audio device, will play without audio";

            return nullptr;
        }

        scheduler.setAudioSink(&audioBufferDevice);

        std::unique_ptr<QAudioSink> sink = std::make_unique<QAudioSink>(audioDevice, format);
        sink->start(&audioBufferDevice);
        return sink;
    };
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    setSurfaceFormat();

    Dream::EventLoop netLoop;
    std::jthread netThread([&netLoop] {
        netLoop.loop();
    });

    DecodeScheduler scheduler{};
    ClientWindow w{ &scheduler, &netLoop };
    setupVideoWidget(scheduler, w);

    // 没有引入回声消除，所以这里默认注释
    // AudioBufferDevice audioBufferDevice{};
    // std::unique_ptr<QAudioSink> sink = setupAudio(scheduler, audioBufferDevice);

    w.show();
    const int result = a.exec();

    scheduler.stop();
    netLoop.quit();

    return result;
}

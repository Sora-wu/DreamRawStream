//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <server/streamServer.h>

#include <print>

using namespace Dream;

namespace {
    constexpr uint32_t AUDIO_QUE_SIZE = 1024;
    constexpr uint32_t VIDEO_QUE_SIZE = 256;
}

StreamServer::StreamServer(EventLoop* loop, const Address& address,
                           std::unique_ptr<Camera> camera, std::unique_ptr<Audio> audio) :
    baseTime_(std::chrono::steady_clock::now()),
    cameraCapturer_(baseTime_),
    audioCapturer_(baseTime_),
    loop_(loop),
    server_(std::make_unique<TcpServer>(loop, address)),
    audioQue_(AUDIO_QUE_SIZE),
    videoQue_(VIDEO_QUE_SIZE) {
    cameraCapturer_.setCamera(std::move(camera));
    audioCapturer_.setAudio(std::move(audio));
    cameraCapturer_.setNextHandler(this);
    audioCapturer_.setNextHandler(this);
}

StreamServer::~StreamServer() {
    stopServer();
}

void StreamServer::startServer(uint32_t threadCount) {
    if (started_.exchange(true)) {
        return;
    }

    server_->setThreadCount(threadCount);
    server_->start();

    // 消费者先于生产者启动，避免采集帧无处可去
    start("stream-push");
    cameraCapturer_.start("capture-video");
    audioCapturer_.start("capture-audio");
}

void StreamServer::stopServer() {
    if (stopped_.exchange(true)) {
        return;
    }

    // 停止生产者
    cameraCapturer_.exit();
    audioCapturer_.exit();
    audioQue_.close();
    videoQue_.close();
    cameraCapturer_.wait();
    audioCapturer_.wait();

    // 停止消费者
    exit();
    wait();
    while (videoQue_.try_pop()) {}
    while (audioQue_.try_pop()) {}

    loop_->quit();
}

void StreamServer::handle(void* data) {
    Frame frame = std::move(*(Frame*)data);

    switch (frame.type) {
    case FrameType::Video:
        videoQue_.push(std::move(frame));
        break;
    case FrameType::Audio:
        audioQue_.push(std::move(frame));
        break;
    default:
        std::println(stderr, "unknown frame type");
        ::exit(1);
    }
}

void StreamServer::run(std::stop_token st) {
    while (!st.stop_requested()) {
        std::optional<Frame> frameOpt = getFrame();
        if (!frameOpt) {
            msleep(1);
            continue;
        }

        const Frame& frame = *frameOpt;
        FrameHeader header{};
        header.type = (uint8_t)frame.type;
        header.size = frame.buffer.size;
        header.pts = frame.pts;
        server_->sendBroadcast((char*)&frame, sizeof(frame));
        server_->sendBroadcast(frame.buffer.data, frame.buffer.size);
    }
}

std::optional<Frame> StreamServer::getFrame() {
    if (videoQue_.empty() && audioQue_.empty()) {
        return std::nullopt;
    }

    if (!videoQue_.empty() && audioQue_.empty()) {
        return videoQue_.pop();
    }

    if (videoQue_.empty() && !audioQue_.empty()) {
        return audioQue_.pop();
    }

    int64_t currentVideoPts = 0;
    int64_t currentAudioPts = 0;
    videoQue_.visit_front([&](const Frame& frame) {
        currentVideoPts = frame.pts;
    });
    audioQue_.visit_front([&](const Frame& frame) {
        currentAudioPts = frame.pts;
    });

    if (currentAudioPts < currentVideoPts) {
        return audioQue_.pop();
    }

    return videoQue_.pop();
}

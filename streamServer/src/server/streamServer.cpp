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

StreamServer::StreamServer(EventLoop* loop, const Address& address) :
    loop_(loop), server_(std::make_unique<TcpServer>(loop, address)),
    audioQue_(AUDIO_QUE_SIZE), videoQue_(VIDEO_QUE_SIZE) {

}

void StreamServer::startServer(uint32_t threadCount) const {
    server_->setThreadCount(threadCount);
    server_->start();
}

void StreamServer::stopServer() const {
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
        server_->sendBroadcast(frame.buffer.data, frame.buffer.size);
        MemoryPool* pool = frame.buffer.pool;
        pool->deallocate(frame.buffer.data, frame.buffer.size);
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
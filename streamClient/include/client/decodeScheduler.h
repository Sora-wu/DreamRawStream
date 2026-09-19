//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <concurrentQueue.hpp>
#include <dataHandler.hpp>
#include <client/structs.h>

#include <vector>
#include <deque>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

class VideoDecoder;
class AudioDecoder;
class IVideoSink;
class IAudioSink;

class DecodeScheduler : public DataHandler {
    struct StreamDecoder {
        std::unique_ptr<VideoDecoder> videoDecoder;
        std::unique_ptr<AudioDecoder> audioDecoder;
    };

public:
    static constexpr uint32_t MAX_STREAM_COUNT = 9;

    DecodeScheduler();
    ~DecodeScheduler() override;

    void setVideoSink(uint32_t streamID, IVideoSink* sink);
    void setAudioSink(uint32_t streamID, IAudioSink* sink);

    void stop();

protected:
    void handle(void* data) override;
    void workerLoop(std::stop_token st);

private:
    bool inReady(uint32_t streamID) const;
    void refreshDecoder(uint32_t streamID, FrameType frameType);

private:
    std::vector<ConcurrentQueue<DecodeFrame>> streamsQues_;
    std::vector<std::jthread> threads_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::deque<uint32_t> readyQue_;
    std::unordered_set<uint32_t> inFlight_;
    std::vector<uint32_t> dropCount_;

    std::array<StreamDecoder, MAX_STREAM_COUNT> decoders_;
    std::array<IVideoSink*, MAX_STREAM_COUNT> videoSinks_;
    std::array<IAudioSink*, MAX_STREAM_COUNT> audioSinks_;
};

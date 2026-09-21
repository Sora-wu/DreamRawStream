//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <client/decodeScheduler.h>
#include <decoder/videoDecoder.h>
#include <decoder/audioDecoder.h>
#include <decoder/IVideoSink.h>
#include <decoder/IAudioSink.h>
#include <client/structs.h>

#include <algorithm>
#include <cassert>

namespace {
    constexpr size_t MAX_FRAMES = 3;
}

DecodeScheduler::DecodeScheduler() {
    const uint32_t threadCountActually = std::clamp(std::thread::hardware_concurrency(), 2u, MAX_STREAM_COUNT);
    threads_.reserve(threadCountActually);
    dropCount_.resize(MAX_STREAM_COUNT);

    for (uint32_t i = 0; i < threadCountActually; ++i) {
        threads_.emplace_back([this](std::stop_token st) {
            workerLoop(st);
        });
    }

    for (uint32_t i = 0; i < MAX_STREAM_COUNT; ++i) {
        ConcurrentQueue<DecodeFrame> que = ConcurrentQueue<DecodeFrame>(MAX_FRAMES);
        streamsQues_.emplace_back(std::move(que));
    }
}

DecodeScheduler::~DecodeScheduler() {}

void DecodeScheduler::setVideoSink(uint32_t streamID, IVideoSink* sink) {
    if (streamID < MAX_STREAM_COUNT) {
        videoSinks_[streamID] = sink;
    }
}

void DecodeScheduler::setAudioSink(IAudioSink* sink) {
    audioSink_ = sink;
}

void DecodeScheduler::setAudioStreamID(uint32_t streamID) {
    std::unique_lock lock(mtx_);
    audioStreamID_ = streamID;
}

void DecodeScheduler::stop() {
    {
        std::unique_lock lock(mtx_);
        readyQue_.clear();
        for (auto& thread : threads_) {
            thread.request_stop();
        }

        for (auto& que : streamsQues_) {
            que.close();
        }

        cv_.notify_all();
    }

    for (auto& thread : threads_) {
        thread.join();
    }
}

void DecodeScheduler::handle(void* data) {
    DecodeFrame frame = std::move(*(DecodeFrame*)data);
    const uint32_t streamID = frame.streamID;

    if (!streamsQues_[streamID].try_push(std::move(frame))) {
        // 预留，方便后面拓展
        ++dropCount_[streamID];
        return;
    }

    std::unique_lock lock(mtx_);
    // 确保所有线程中，不存在相同的流，这样避免有的数据后到但是先处理
    if (inFlight_.contains(streamID) || inReady(streamID)) {
        return;
    }
    readyQue_.push_back(streamID);
    cv_.notify_one();
}

void DecodeScheduler::workerLoop(std::stop_token st) {
    while (!st.stop_requested()) {
        uint32_t streamID = 0;
        {
            std::unique_lock lock(mtx_);
            cv_.wait(lock, [&] { return st.stop_requested() || !readyQue_.empty(); });
            if (readyQue_.empty()) {
                break;
            }

            streamID = readyQue_.front();
            assert(streamID < MAX_STREAM_COUNT);
            readyQue_.pop_front();
            inFlight_.insert(streamID);
        }

        if (std::optional<DecodeFrame> headerOpt = streamsQues_[streamID].pop()) {
            DecodeFrame frame = std::move(*headerOpt);
            refreshDecoder(streamID, frame.frame.type);
            if (frame.frame.type == FrameType::VIDEO) {
                decoders_[streamID].videoDecoder->decode(frame.frame.buffer.data, frame.frame.buffer.size, frame.frame.pts,
                    [this, streamID](const VideoFrame& videoFrame) {
                        if (videoSinks_[streamID]) {
                            videoSinks_[streamID]->onVideoFrame(videoFrame);
                        }
                    });
            }
            else if (frame.frame.type == FrameType::AUDIO) {
                // 当前取到的流id是否与界面选择的id是一致的
                bool isSelected = false;
                {
                    std::unique_lock lock(mtx_);
                    isSelected = audioStreamID_ == streamID;
                }

                if (isSelected) {
                    decoders_[streamID].audioDecoder->decode(frame.frame.buffer.data, frame.frame.buffer.size, frame.frame.pts,
                        [this](const AudioFrame& audioFrame) {
                            if (audioSink_) {
                                audioSink_->onAudioFrame(audioFrame);
                            }
                        });
                }
            }
        }

        {
            std::unique_lock lock(mtx_);
            inFlight_.erase(streamID);
            if (!streamsQues_[streamID].empty()) {
                // 如果当前流的队列不为空，那么就绪队列接着就绪
                readyQue_.push_back(streamID);
                cv_.notify_one();
            }
        }
    }
}

bool DecodeScheduler::inReady(uint32_t streamID) const {
    return std::ranges::find(readyQue_, streamID) != readyQue_.end();
}

void DecodeScheduler::refreshDecoder(uint32_t streamID, FrameType frameType) {
    if (!decoders_[streamID].videoDecoder && frameType == FrameType::VIDEO) {
        decoders_[streamID].videoDecoder = std::make_unique<VideoDecoder>();
    }

    if (!decoders_[streamID].audioDecoder && frameType == FrameType::AUDIO) {
        decoders_[streamID].audioDecoder = std::make_unique<AudioDecoder>();
    }
}

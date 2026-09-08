//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <client/decodeScheduler.h>

#include <algorithm>
#include <client/structs.h>
#include <cassert>

DecodeScheduler::DecodeScheduler() {
    const uint32_t threadCountActually = std::clamp(std::thread::hardware_concurrency(), 2u, MAX_STREAM_COUNT);
    threads_.reserve(threadCountActually);
    dropCount_.resize(MAX_STREAM_COUNT);

    for (uint32_t i = 0; i < threadCountActually; ++i) {
        threads_.emplace_back([this](std::stop_token st) {
            workerLoop(st);
        });
    }
}

DecodeScheduler::~DecodeScheduler() {
    stop();
}

void DecodeScheduler::stop() {
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
            // TODO: decode
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

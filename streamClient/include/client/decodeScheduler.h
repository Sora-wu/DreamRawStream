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

class DecodeScheduler : public DataHandler {
    static constexpr uint32_t MAX_STREAM_COUNT = 9;

public:
    DecodeScheduler();
    ~DecodeScheduler() override;

    void stop();

protected:
    void handle(void* data) override;
    void workerLoop(std::stop_token st);

private:
    bool inReady(uint32_t streamID) const;

private:
    std::array<ConcurrentQueue<DecodeFrame>, MAX_STREAM_COUNT> streamsQues_;
    std::vector<std::jthread> threads_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::deque<uint32_t> readyQue_;
    std::unordered_set<uint32_t> inFlight_;
    std::vector<uint32_t> dropCount_;
};

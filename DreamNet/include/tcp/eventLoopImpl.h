//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/eventLoop.h>
#include <common/concurrentQueue.hpp>
#include <tcp/timerQueue.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <chrono>

namespace Dream::detail {
    class Channel;
    class Poller;

    class EventLoopImpl {
    public:
        EventLoopImpl();

        // 从公共 EventLoop 获取内部 Impl（仅供 detail 内部代码使用）
        [[nodiscard]] static EventLoopImpl* from(const Dream::EventLoop* loop) {
            return loop->impl_;
        }

        void loop();
        void quit();

        void wakeup() const;
        void runInLoop(Functor func);
        uint64_t runAfter(Functor func, std::chrono::milliseconds delay) const;
        uint64_t runAfter(Functor func, std::chrono::seconds delay) const;
        void cancelAfter(uint64_t tid) const;
        void queueInLoop(Functor func);

        [[nodiscard]] std::thread::id getThreadId() const;
        [[nodiscard]] bool isInLoopThread() const;

        ////////////////////Impl特有////////////////////////
        void updateChannel(Channel* channel) const;
        void removeChannel(Channel* channel) const;
        [[nodiscard]] uint32_t getLoad() const;

    private:
        void processPendingFunctors();
        void onWakeUp() const;

    private:
        int evtFD_ = -1;
        std::thread::id threadId_{};
        std::unique_ptr<Poller> poller_;

        std::unique_ptr<Channel> wakeupChannel_;
        std::vector<Channel*> activeChannels_;

        std::atomic_bool isQuit_ = false;

        std::atomic_bool callingPendingFunctors_ = false;
        ConcurrentQueue<Functor> pendingFunctors_;

        std::unique_ptr<TimerQueue> timerQueue_;
    };
}

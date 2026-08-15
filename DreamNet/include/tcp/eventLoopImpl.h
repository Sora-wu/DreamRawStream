//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/eventLoop.h>
#include <common/concurrentQueue.hpp>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

namespace Dream::detail {
    class Channel;
    class Poller;

    class EventLoopImpl {
    public:
        EventLoopImpl();
        ~EventLoopImpl();

        // 从公共 EventLoop 获取内部 Impl（仅供 detail 内部代码使用）
        [[nodiscard]] static EventLoopImpl* from(const EventLoop* loop) {
            return loop->impl_.get();
        }

        void loop();
        void quit();

        void wakeup() const;
        void runInLoop(EventLoop::Functor func);
        void queueInLoop(EventLoop::Functor func);

        [[nodiscard]] std::thread::id getThreadId() const;
        [[nodiscard]] bool isInLoopThread() const;

        ////////////////////Impl特有////////////////////////
        void updateChannel(Channel* channel) const;
        void removeChannel(Channel* channel) const;

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
        ConcurrentQueue<EventLoop::Functor> pendingFunctors_;
    };
} // namespace Dream::detail

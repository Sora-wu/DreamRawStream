//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/callbacks.h>
#include <thread>

namespace Dream::detail {
    class EventLoopImpl;
}

namespace Dream {
    class EventLoop {
    public:
        EventLoop();
        ~EventLoop();
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;

        void loop() const;
        void quit() const;

        void wakeup() const;
        void runInLoop(Functor func) const;
        void runAfter(Functor func, std::chrono::milliseconds delay) const;
        void runAfter(Functor func, std::chrono::seconds delay) const;
        void cancelAfter(uint64_t tid) const;
        void queueInLoop(Functor func) const;

        [[nodiscard]] std::thread::id getThreadId() const;
        [[nodiscard]] bool isInLoopThread() const;

    private:
        friend class detail::EventLoopImpl;
        detail::EventLoopImpl* impl_ = nullptr;
    };
}

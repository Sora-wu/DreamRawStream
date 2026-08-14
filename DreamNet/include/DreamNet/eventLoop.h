//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <memory>
#include <functional>
#include <thread>

class Channel;
class Acceptor;

namespace Dream {
    class EventLoop {
    public:
        using Functor = std::function<void()>;

        EventLoop();
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;

        void loop() const;
        void quit() const;

        void wakeup() const;
        void runInLoop(Functor func) const;
        void queueInLoop(Functor func) const;

        [[nodiscard]] std::thread::id getThreadId() const;
        [[nodiscard]] bool isInLoopThread() const;

    private:
        friend class ::Channel;
        friend class ::Acceptor;
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <thread>

namespace Dream {
    class EventLoop;

    namespace detail {
        class EventLoopThread {
        public:
            explicit EventLoopThread(const std::string& threadName = {});
            ~EventLoopThread();
            EventLoopThread(const EventLoopThread&) = delete;
            EventLoopThread& operator=(const EventLoopThread&) = delete;

            void start();
            [[nodiscard]] Dream::EventLoop* getEventLoop() const;

        private:
            std::jthread thread_;
            Dream::EventLoop* loop_ = nullptr;
            const std::string threadName_;
        };
    }
}



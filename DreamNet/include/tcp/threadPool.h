//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <vector>
#include <cstdint>

namespace Dream {
    class EventLoop;

    namespace detail {
        class EventLoopThread;
        class EventLoopImpl;

        class ThreadPool {
        public:
            explicit ThreadPool(EventLoop* baseLoop) : baseLoop_(baseLoop) {}

            ThreadPool(const ThreadPool&) = delete;
            ThreadPool& operator=(const ThreadPool&) = delete;

            void start();
            void setThreadNum(uint32_t threadNum = 0);
            [[nodiscard]] EventLoop* getNextLoop() const;

        private:
            EventLoop* baseLoop_ = nullptr;
            std::vector<EventLoopThread*> threads_;
            uint32_t threadNum_ = 0;
        };
    }
}
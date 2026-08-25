//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/callbacks.h>
#include <tcp/channel.h>

#include <cstdint>
#include <chrono>
#include <set>
#include <atomic>

namespace Dream::detail {
    class Channel;

    using TimePoint = std::chrono::steady_clock::time_point;
    struct Timer {
        uint64_t id = 0;
        TimePoint expire;                                   // 绝对到期时间
        std::chrono::milliseconds delay;                    // 相对到期时间
        std::chrono::milliseconds interval;                 // 周期性 0=一次性 >0周期
        Functor callback = nullptr;

        bool operator<(const Timer& other) const {
            if (expire != other.expire) {
                return expire < other.expire;
            }
            return id < other.id;                           // 兜底
        }
    };

    class TimerQueue {
    public:
        explicit TimerQueue(EventLoopImpl* loop);
        ~TimerQueue();
        TimerQueue(const TimerQueue&) = delete;
        TimerQueue& operator=(const TimerQueue&) = delete;

        uint64_t addTimer(Functor callback, std::chrono::milliseconds delay, std::chrono::milliseconds interval = {});
        uint64_t addTimer(Functor callback, std::chrono::seconds delay, std::chrono::seconds interval = {});

    private:
        void handleRead();
        void armTimer() const;

        uint64_t addTimerPri(Functor callback, std::chrono::milliseconds delay, std::chrono::milliseconds interval);
        uint64_t addTimerPri(Functor callback, std::chrono::seconds delay, std::chrono::seconds interval);

    private:
        EventLoopImpl* loop_ = nullptr;
        std::atomic<uint64_t> currentID_ = 0;
        int tfd_ = -1;
        std::unique_ptr<Channel> timerChannel_;

        std::multiset<Timer> timers_;
    };
}

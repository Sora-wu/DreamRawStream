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

namespace Dream::detail {
    class EventLoopImpl;
    class Channel;

    using TimePoint = std::chrono::steady_clock::time_point;
    struct Timer {
        uint64_t id = 0;
        TimePoint expire;                                   // 绝对到期时间
        std::chrono::milliseconds delay;                    // 相对到期时间
        std::chrono::milliseconds interval;                 // 周期性 0=一次性 >0周期
        Functor callback = nullptr;

        bool operator<(const Timer& other) const {
            return expire < other.expire;
        }
    };

    class TimerQueue {
    public:
        explicit TimerQueue(EventLoopImpl* loop) : loop_(loop) {}
        TimerQueue(const TimerQueue&) = delete;
        TimerQueue& operator=(const TimerQueue&) = delete;

        uint64_t addTimer(Functor callback, std::chrono::milliseconds delay, std::chrono::milliseconds interval = {});
        uint64_t addTimer(Functor callback, std::chrono::seconds delay, std::chrono::seconds interval = {});

    private:
        void handleRead();
        void resetTimerEvent();
        void registerTimer(const Timer& timer);

        template <typename Period>
        uint64_t addTimerPri(Functor callback, Period delay, Period interval);

    private:
        EventLoopImpl* loop_ = nullptr;
        uint64_t currentID_ = 0;
        int tfd_ = -1;
        std::unique_ptr<Channel> timerChannel_;

        std::multiset<Timer> timers_;
    };

    template <typename Period>
    uint64_t TimerQueue::addTimerPri(Functor callback, Period delay, Period interval) {
        TimePoint now = std::chrono::steady_clock::now();
        Timer timer{};
        timer.id = currentID_++;
        timer.expire = now + delay;
        timer.interval = interval;
        timer.delay = delay;
        timer.callback = std::move(callback);
        timers_.insert(timer);

        resetTimerEvent();
        registerTimer(*timers_.begin());

        return timer.id;
    }
}

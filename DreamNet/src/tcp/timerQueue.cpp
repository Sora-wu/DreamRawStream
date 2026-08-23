//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/timerQueue.h>
#include <common/log.hpp>

#include <unistd.h>
#include <sys/timerfd.h>

using namespace Dream::detail;

namespace {
    // 将任意 duration 转换为 timespec
    template <typename Rep, typename Period>
    timespec toTimespec(const std::chrono::duration<Rep, Period>& dur) {
        std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur);
        timespec ts{};
        ts.tv_sec  = static_cast<time_t>(ns.count() / 1'000'000'000LL);
        ts.tv_nsec = static_cast<long>(ns.count() % 1'000'000'000LL);
        return ts;
    }
}

uint64_t TimerQueue::addTimer(Functor callback, std::chrono::milliseconds delay, std::chrono::milliseconds interval) {
    return addTimerPri(std::move(callback), delay, interval);
}

uint64_t TimerQueue::addTimer(Functor callback, std::chrono::seconds delay, std::chrono::seconds interval) {
    return addTimerPri(std::move(callback), delay, interval);
}

void TimerQueue::handleRead() {
    uint64_t exp = 0;
    const ssize_t n = read(tfd_, &exp, 8);
    if (n != sizeof(uint64_t)) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("error, read {}", std::strerror(errno));
        }
        return;
    }

    TimePoint now = std::chrono::steady_clock::now();
    while (!timers_.empty() && timers_.begin()->expire <= now) {
        Timer t = std::move(timers_.extract(timers_.begin()).value());
        t.callback();

        if (t.interval.count() > 0) {
            t.expire = now + t.interval;
            registerTimer(t);
        }
    }
}

void TimerQueue::resetTimerEvent() {
    timerChannel_.reset();
}

void TimerQueue::registerTimer(const Timer& timer) {
    itimerspec its{};
    its.it_value = toTimespec(timer.delay);
    its.it_interval = toTimespec(timer.interval);
    tfd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (tfd_ < 0) {
        LOG_ERROR("error, timerfd_create failed: {}", std::strerror(errno));
        return;
    }
    timerChannel_ = std::make_unique<Channel>(loop_, tfd_);
    timerChannel_->setOnReadEvent([this] {
        handleRead();
    });
}

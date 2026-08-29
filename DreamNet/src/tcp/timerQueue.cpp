//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/timerQueue.h>
#include <common/log.hpp>
#include <tcp/eventLoopImpl.h>

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

    template <typename Period>
    Timer createTimer(Dream::Functor callback, Period delay, Period interval, uint64_t id) {
        TimePoint now = std::chrono::steady_clock::now();
        Timer timer{};
        timer.id = id;
        timer.expire = now + delay;
        timer.interval = interval;
        timer.delay = delay;
        timer.callback = std::move(callback);

        return timer;
    }
}

TimerQueue::TimerQueue(EventLoopImpl* loop) : loop_(loop) {
    tfd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (tfd_ < 0) {
        LOG_ERROR("error, timerfd_create failed: {}", std::strerror(errno));
        return;
    }
    timerChannel_ = std::make_unique<Channel>(loop_, tfd_);
    timerChannel_->setOnReadEvent([this] { handleRead(); });
    timerChannel_->enableReading();
}

TimerQueue::~TimerQueue() {
    if (tfd_ >= 0) {
        ::close(tfd_);
    }
}

uint64_t TimerQueue::addTimer(Functor callback, std::chrono::milliseconds delay, std::chrono::milliseconds interval) {
    return addTimerPri(std::move(callback), delay, interval);
}

uint64_t TimerQueue::addTimer(Functor callback, std::chrono::seconds delay, std::chrono::seconds interval) {
    return addTimerPri(std::move(callback), delay, interval);
}

void TimerQueue::removeTimer(uint64_t id) {
    bool isRemoved = false;
    for (auto it = timers_.begin(); it != timers_.end(); /**/) {
        if (it->id == id) {
            it = timers_.erase(it);
            isRemoved = true;
        } else {
            ++it;
        }
    }

    if (isRemoved) {
        armTimer();
    }
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
            t.expire += t.interval;
            timers_.insert(std::move(t));
        }
    }

    armTimer();
}

void TimerQueue::armTimer() const {
    if (timers_.empty()) {
        itimerspec its{};
        timerfd_settime(tfd_, 0, &its, nullptr);
        return;
    }

    auto delta = timers_.begin()->expire - std::chrono::steady_clock::now();
    if (delta < std::chrono::microseconds(100)) {
        delta = std::chrono::microseconds(100);
    }

    itimerspec its{};
    its.it_value = toTimespec(delta);
    timerfd_settime(tfd_, 0, &its, nullptr);
}

uint64_t TimerQueue::addTimerPri(Functor callback, std::chrono::milliseconds delay, std::chrono::milliseconds interval) {
    Timer timer = std::move(createTimer(std::move(callback), delay, interval, currentID_++));
    loop_->runInLoop([this, timer = std::move(timer)]() mutable {
        timers_.insert(timer);
        armTimer();
    });

    return timer.id;
}

uint64_t TimerQueue::addTimerPri(Functor callback, std::chrono::seconds delay, std::chrono::seconds interval) {
    Timer timer = std::move(createTimer(std::move(callback), delay, interval, currentID_++));
    loop_->runInLoop([this, timer = std::move(timer)]() mutable {
        timers_.insert(timer);
        armTimer();
    });

    return timer.id;
}

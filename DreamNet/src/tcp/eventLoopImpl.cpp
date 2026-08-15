//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/eventLoopImpl.h>
#include <tcp/channel.h>
#include <tcp/poller.h>
#include <common/define.h>

#include <sys/eventfd.h>
#include <unistd.h>
#include <deque>

namespace {
    constexpr int TIMEOUT = 1e4;
}

using namespace Dream::detail;

EventLoopImpl::EventLoopImpl() :
    evtFD_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
    threadId_(std::this_thread::get_id()) {
    if (evtFD_) {
        LOG_FATAL("eventfd error: {}", strerror(errno));
    }

    poller_.reset(Poller::getDefaultPoller());
    wakeupChannel_ = std::make_unique<Channel>(this, evtFD_);
    if (!wakeupChannel_) {
        LOG_FATAL("wakeup channel is null");
    }
    wakeupChannel_->enableReading(); // 设置可读标志后并设置到epoll
    wakeupChannel_->setOnReadEvent([this] { onWakeUp(); });
}

EventLoopImpl::~EventLoopImpl() {
}

void EventLoopImpl::loop() {
    while (!isQuit_) {
        activeChannels_.clear();
        poller_->poll(activeChannels_, TIMEOUT);
        for (Channel* channel : activeChannels_) {
            channel->handleEvent();
        }
        processPendingFunctors();
    }

    LOG_INFO("Event Loop exit");
}

void EventLoopImpl::quit() {
    isQuit_ = true;
}

void EventLoopImpl::wakeup() const {
    const uint64_t one = 1;
    const uint32_t res = write(evtFD_, &one, sizeof one);
    if (res != sizeof(one)) {
        LOG_ERROR("wakeup write {} instead of {}", res, sizeof(one));
    }
}

void EventLoopImpl::runInLoop(EventLoop::Functor func) {
    if (isInLoopThread()) {
        func();
        return;
    }

    queueInLoop(func);
}

void EventLoopImpl::queueInLoop(EventLoop::Functor func) {
    pendingFunctors_.push(std::move(func));

    if (!isInLoopThread() || callingPendingFunctors_) {
        // 如果非当前线程，或者正在执行pendingFunctors_
        wakeup();
    }
}

std::thread::id EventLoopImpl::getThreadId() const {
    return threadId_;
}

bool EventLoopImpl::isInLoopThread() const {
    return threadId_ == std::this_thread::get_id();
}

void EventLoopImpl::updateChannel(Channel* channel) const {
    poller_->updateChannel(channel);
}

void EventLoopImpl::removeChannel(Channel* channel) const {
    poller_->removeChannel(channel);
}

void EventLoopImpl::processPendingFunctors() {
    std::deque<EventLoop::Functor> tmp;

    callingPendingFunctors_ = true;
    pendingFunctors_.pop_all(tmp);
    for (auto& func : tmp) {
        func();
    }
    callingPendingFunctors_ = false;
}

void EventLoopImpl::onWakeUp() const {
    uint64_t one{};
    int res = read(evtFD_, &one, sizeof(one));
    if (res != sizeof(one)) {
        LOG_ERROR("read {} instead of {}", res, sizeof(one));
    }
}

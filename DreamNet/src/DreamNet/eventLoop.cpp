//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/eventLoop.h>
#include <tcp/eventLoopImpl.h>
#include <tcp/channel.h>
#include <tcp/poller.h>

using namespace Dream;

EventLoop::EventLoop() : impl_(new detail::EventLoopImpl) {}

EventLoop::~EventLoop() {
    if (impl_) {
        delete impl_;
        impl_ = nullptr;
    }
}

void EventLoop::loop() const {
    impl_->loop();
}

void EventLoop::quit() const {
    impl_->quit();
}

void EventLoop::wakeup() const {
    impl_->wakeup();
}

void EventLoop::runInLoop(Functor func) const {
    impl_->runInLoop(std::move(func));
}

void EventLoop::runAfter(Functor func, std::chrono::milliseconds delay) const {
    impl_->runAfter(std::move(func), delay);
}

void EventLoop::runAfter(Functor func, std::chrono::seconds delay) const {
    impl_->runAfter(std::move(func), delay);
}

void EventLoop::cancelAfter(uint64_t tid) const {
    impl_->cancelAfter(tid);
}

void EventLoop::queueInLoop(Functor func) const {
    impl_->queueInLoop(std::move(func));
}

std::thread::id EventLoop::getThreadId() const {
    return impl_->getThreadId();
}

bool EventLoop::isInLoopThread() const {
    return impl_->isInLoopThread();
}

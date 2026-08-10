//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/eventLoop.h>
#include <tcp/eventLoopImpl.h>
#include <common/define.h>

using namespace Dream;

EventLoop::EventLoop() : impl_(std::make_unique<Impl>()) {
    if (!impl_) {
        LOG_FATAL("EventLoop::EventLoop(): impl_ is null");
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

void EventLoop::queueInLoop(Functor func) const {
    impl_->queueInLoop(std::move(func));
}

std::thread::id EventLoop::getThreadId() const {
    return impl_->getThreadId();
}

bool EventLoop::isInLoopThread() const {
    return impl_->isInLoopThread();
}

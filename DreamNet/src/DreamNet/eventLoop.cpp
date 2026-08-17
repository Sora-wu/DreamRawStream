//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/eventLoop.h>
#include <tcp/eventLoopImpl.h>
#include <tcp/channel.h>
#include <tcp/poller.h>
#include <common/define.h>

using namespace Dream;

EventLoop::EventLoop() : impl_(new detail::EventLoopImpl(this)) {}

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

void EventLoop::queueInLoop(Functor func) const {
    impl_->queueInLoop(std::move(func));
}

std::thread::id EventLoop::getThreadId() const {
    return impl_->getThreadId();
}

bool EventLoop::isInLoopThread() const {
    return impl_->isInLoopThread();
}

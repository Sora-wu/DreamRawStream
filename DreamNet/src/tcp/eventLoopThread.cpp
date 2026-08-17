//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/eventLoopThread.h>
#include <DreamNet/eventLoop.h>
#include <tcp/epollPoller.h>
#include <tcp/eventLoopImpl.h>
#include <tcp/channel.h>
#include <common/log.hpp>

using namespace Dream;

detail::EventLoopThread::EventLoopThread(const std::string& threadName) : loop_(new EventLoop), threadName_(threadName) {}

detail::EventLoopThread::~EventLoopThread() {
    if (loop_) {
        delete loop_;
        loop_ = nullptr;
    }
}

void detail::EventLoopThread::start() {
    thread_ = std::jthread([this] {
        loop_->loop();
        if (!threadName_.empty()) {
            LOG_INFO("thread {} exit", threadName_);
        }
    });
}

EventLoop* detail::EventLoopThread::getEventLoop() const {
    return loop_;
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/threadPool.h>
#include <tcp/eventLoopThread.h>
#include <tcp/eventLoopImpl.h>

using namespace Dream;

void detail::ThreadPool::start() {
    for (uint32_t i = 0; i < threadNum_; ++i) {
        EventLoopThread* t = new EventLoopThread;
        t->start();
        threads_.push_back(t);
    }
}

void detail::ThreadPool::setThreadNum(uint32_t threadNum) {
    threadNum_ = threadNum;
}

EventLoop* detail::ThreadPool::getNextLoop() const {
    uint32_t minLoad = std::numeric_limits<uint32_t>::max();
    EventLoop* resLoop = baseLoop_;

    for (auto eventLoopThread : threads_) {
        EventLoop* loop = eventLoopThread->getEventLoop();
        EventLoopImpl* loopImpl = EventLoopImpl::from(loop);
        if (loopImpl->getLoad() < minLoad) {
            minLoad = loopImpl->getLoad();
            resLoop = loop;
        }
    }

    return resLoop;
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/eventLoop.h>
#include <common/concurrentQueue.hpp>

#include <atomic>

class Channel;
class Poller;

class Dream::EventLoop::Impl {
public:
    Impl();
    ~Impl();

    void loop();
    void quit();

    void wakeup() const;
    void runInLoop(Functor func);
    void queueInLoop(Functor func);

    [[nodiscard]] std::thread::id getThreadId() const;
    [[nodiscard]] bool isInLoopThread() const;

    ////////////////////Impl特有////////////////////////
    void updateChannel(Channel* channel) const;
    void removeChannel(Channel* channel) const;

private:
    void processPendingFunctors();
    void onWakeUp() const;

private:
    int evtFD_ = -1;
    std::thread::id threadId_{};
    std::unique_ptr<Poller> poller_;

    std::unique_ptr<Channel> wakeupChannel_;
    std::vector<Channel*> activeChannels_;

    std::atomic_bool isQuit_ = false;

    std::atomic_bool callingPendingFunctors_ = false;
    ConcurrentQueue<Functor> pendingFunctors_;
};

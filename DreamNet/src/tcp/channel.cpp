//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/channel.h>
#include <tcp/eventLoopImpl.h>

using namespace Dream::detail;

Channel::Channel(EventLoopImpl* loop, int fd) : loopImpl_(loop), fd_(fd) {}

Channel::~Channel() {
    fd_ = -1;
}

int Channel::getFd() const {
    return fd_;
}

ChannelStatus Channel::getStatus() const {
    return status_;
}

void Channel::setStatus(ChannelStatus status) {
    status_ = status;
}

bool Channel::isNoneEvent() const {
    return listenEvent_ == EVENT_NONE;
}

bool Channel::isReading() const {
    return listenEvent_ & (EVENT_READ | EVENT_PRI);
}

bool Channel::isWriting() const {
    return listenEvent_ & EVENT_WRITE;
}

void Channel::enableReading() {
    listenEvent_ |= EVENT_READ;
    update();
}

void Channel::enableWriting() {
    listenEvent_ |= EVENT_WRITE;
    update();
}

void Channel::disableReading() {
    listenEvent_ &= ~EVENT_READ;
    update();
}

void Channel::disableWriting() {
    listenEvent_ &= ~EVENT_WRITE;
    update();
}

void Channel::disableAll() {
    listenEvent_ = EVENT_NONE;
    update();
}

uint32_t Channel::getListenEvent() const {
    return listenEvent_;
}

void Channel::update() {
    loopImpl_->updateChannel(this);
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    isTied_ = true;
}

void Channel::setActualEvent(uint32_t event) {
    actualEvent_ = event;
}

void Channel::setOnReadEvent(Functor func) {
    onReadEvent_ = std::move(func);
}

void Channel::setOnWriteEvent(Functor func) {
    onWriteEvent_ = std::move(func);
}

void Channel::setOnCloseEvent(Functor func) {
    onCloseEvent_ = std::move(func);
}

void Channel::setOnErrorEvent(Functor func) {
    onErrorEvent_ = std::move(func);
}

void Channel::handleEvent() const {
    if (isTied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard) {
            handleEventWithGuard();
        }
        return;
    }

    handleEventWithGuard();
}

void Channel::handleEventWithGuard() const {
    if ((actualEvent_ & EVENT_HUP) && !(actualEvent_ & EVENT_READ)) {
        if (onCloseEvent_) {
            onCloseEvent_();
        }
        return;
    }

    if (actualEvent_ & EVENT_ERROR) {
        if (onErrorEvent_) {
            onErrorEvent_();
        }
        return;
    }

    if (actualEvent_ & (EVENT_READ | EVENT_PRI)) {
        if (onReadEvent_) {
            onReadEvent_();
        }
        return;
    }

    if (actualEvent_ & EVENT_WRITE) {
        if (onWriteEvent_) {
            onWriteEvent_();
        }
    }
}

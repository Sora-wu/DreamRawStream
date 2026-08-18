//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/epollPoller.h>
#include <common/define.h>
#include <tcp/channel.h>

using namespace Dream::detail;

namespace {
    constexpr uint32_t MAX_EVENT = 512;

    // 适配器
    uint32_t convertEpollEvents(uint32_t epollEvents) {
        uint32_t event = Channel::EVENT_NONE;

        if (epollEvents & EPOLLIN)  event |= Channel::EVENT_READ;
        if (epollEvents & EPOLLPRI) event |= Channel::EVENT_PRI;
        if (epollEvents & EPOLLOUT) event |= Channel::EVENT_WRITE;
        if (epollEvents & EPOLLERR) event |= Channel::EVENT_ERROR;
        if (epollEvents & EPOLLHUP) event |= Channel::EVENT_HUP;

        return event;
    }
}

EpollPoller::EpollPoller() :
    epFD_(epoll_create1(EPOLL_CLOEXEC)),
    events_(MAX_EVENT) {
    if (epFD_ < 0) {
        LOG_FATAL("epoll_create1 error: {}", strerror(errno));
    }
}

void EpollPoller::poll(std::vector<Channel*>& activeChannels, int timeout) {
    const int nfds = epoll_wait(epFD_, events_.data(), (int)events_.size(), timeout);
    int saveErrno = errno;
    if (nfds > 0) {
        if (activeChannels.capacity() <= nfds) {
            activeChannels.reserve(nfds);
        }
        fillActiveChannel(nfds, activeChannels);
        return;
    }

    if (nfds < 0 && errno != EINTR) {
        LOG_ERROR("epoll_wait error: {}", strerror(saveErrno));
    }
}

void EpollPoller::updateChannel(Channel* channel) {
    if (channel->isNoneEvent()) {
        removeChannel(channel);
        return;
    }

    const ChannelStatus status = channel->getStatus();
    int op = status == ChannelStatus::IN_LOOP ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    update(op, channel);

    if (status == ChannelStatus::OUT_LOOP) {
        channel->setStatus(ChannelStatus::IN_LOOP);
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    update(EPOLL_CTL_DEL, channel);
    channel->setStatus(ChannelStatus::OUT_LOOP);
}

uint32_t EpollPoller::getLoad() {
    return load_;
}

void EpollPoller::fillActiveChannel(int nfds, std::vector<Channel*>& activeChannels) const {
    for (int i = 0; i < nfds; ++i) {
        const epoll_event& ev = events_[i];
        Channel* channel = (Channel*)ev.data.ptr;
        channel->setActualEvent(convertEpollEvents(ev.events));
        activeChannels.push_back(channel);
    }
}

void EpollPoller::update(int op, Channel* channel) {
    const uint32_t event = channel->getListenEvent();
    const int fd = channel->getFd();
    epoll_event ev{};
    ev.events = event;
    ev.data.ptr = channel;

    if (epoll_ctl(epFD_, op, fd, &ev) < 0) {
        if (op == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll ctl error: {}", strerror(errno));
        }
        else {
            LOG_FATAL("epoll ctl(add/mod) error: {}", strerror(errno));
        }

        return;
    }

    if (op == EPOLL_CTL_ADD) {
        ++load_;
    }
    if (op == EPOLL_CTL_DEL) {
        --load_;
    }
}

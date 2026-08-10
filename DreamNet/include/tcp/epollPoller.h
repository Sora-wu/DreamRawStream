//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <tcp/poller.h>

#include <sys/epoll.h>

class EpollPoller final : public Poller {
public:
    EpollPoller();

    void poll(std::vector<Channel*>& activeChannels, int timeout) override;

    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    void fillActiveChannel(int nfds, std::vector<Channel*>& activeChannels) const;
    void update(int op, Channel* channel) const;

private:
    int epFD_ = -1;
    std::vector<epoll_event> events_;
};

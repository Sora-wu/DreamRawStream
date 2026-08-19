//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <tcp/poller.h>

#include <sys/epoll.h>
#include <vector>

namespace Dream::detail {
    class EpollPoller final : public Poller {
    public:
        EpollPoller();
        ~EpollPoller();

        void poll(std::vector<Channel*>& activeChannels, int timeout) override;

        void updateChannel(Channel* channel) override;
        void removeChannel(Channel* channel) override;
        [[nodiscard]] uint32_t getLoad() override;

    private:
        void fillActiveChannel(int nfds, std::vector<Channel*>& activeChannels) const;
        void update(int op, Channel* channel);

    private:
        int epFD_ = -1;
        std::vector<epoll_event> events_;
        uint32_t load_ = 0;             // 当前的epoll跑了多少个fd（负载）
    };
} // namespace Dream::detail

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <vector>
#include <cstdint>

namespace Dream::detail {
    class Channel;

    class Poller {
    public:
        virtual ~Poller() = default;
        static Poller* getDefaultPoller();

        virtual void poll(std::vector<Channel*>& activeChannels, int timeout) = 0;

        virtual void updateChannel(Channel* channel) = 0;
        virtual void removeChannel(Channel* channel) = 0;
        [[nodiscard]] virtual uint32_t getLoad() = 0;
    };
} // namespace Dream::detail

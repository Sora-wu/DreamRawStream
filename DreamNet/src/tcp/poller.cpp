//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/poller.h>
#include <tcp/epollPoller.h>

Poller* Poller::getDefaultPoller() {
#ifdef PLATFORM_LINUX
    return new EpollPoller;
#endif

    return nullptr;
}

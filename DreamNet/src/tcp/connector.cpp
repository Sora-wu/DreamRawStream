//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/connector.h>
#include <tcp/channel.h>
#include <common/log.hpp>

#include <unistd.h>
#include <cstring>

using namespace Dream;

namespace {
    constexpr uint32_t MAX_RETRIES = 5;
}

detail::Connector::Connector(EventLoopImpl* loop, const Address& addr) :
    loop_(loop),
    addr_(addr),
    socket_(SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK),
    channel_(std::make_unique<Channel>(loop, socket_.getFd())) {
    channel_->setOnWriteEvent([this]{ handleWrite(); });
}

void detail::Connector::start() {
    uint32_t attempt = 0;
    int sockfd = socket_.getFd();

    while (attempt < MAX_RETRIES) {
        int res = socket_.connect(addr_);
        if (res == 0) {
            if (newConnectionCallback_) {
                newConnectionCallback_(socket_.getFd(), addr_);
                return;
            }

            LOG_WARN("the connect callback is not set yet, will close new client by default");
            return;
        }
        if (res == -1 && errno == EINPROGRESS) {
            state_ = State::CONNECTING;
            channel_->enableWriting();
            return;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            ++attempt;
            std::this_thread::sleep_for(std::chrono::milliseconds(retryIntervalMillis_));
            ::close(sockfd);
            continue;
        }

        // 其他错误
        LOG_ERROR("connect failed: {}", strerror(errno));
    }

    LOG_ERROR("failed to connect to server");
}

void detail::Connector::stop() const {
    channel_->disableAll();
    ::close(socket_.getFd());
}

void detail::Connector::setRetryInterval(uint32_t retryIntervalMillis) {
    retryIntervalMillis_ = retryIntervalMillis;
}

void detail::Connector::setNewConnectionCallback(NewConnectionCallback newConnectionCallback) {
    newConnectionCallback_ = std::move(newConnectionCallback);
}

void detail::Connector::handleWrite() {
    int error = 0;
    socklen_t len = sizeof(error);
    const int sockfd = socket_.getFd();

    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        LOG_ERROR("getsockopt failed: {}", strerror(errno));
        return;
    }

    if (error) {
        LOG_ERROR("getsockopt failed: {}", error);
        return;
    }

    state_ = State::CONNECTED;
    channel_->disableAll();
    if (newConnectionCallback_) {
        newConnectionCallback_(sockfd, addr_);
        return;
    }

    LOG_WARN("the connect callback is not set yet, will close new client by default");
}

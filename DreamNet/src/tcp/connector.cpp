//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <tcp/connector.h>
#include <tcp/channel.h>
#include <common/log.hpp>

#include <cstring>
#include <unistd.h>

#include "tcp/eventLoopImpl.h"

using namespace Dream;

namespace {
    constexpr uint32_t MAX_RETRIES = 5;
}

detail::Connector::Connector(EventLoopImpl* loop, const Address& addr) :
    loop_(loop),
    addr_(addr),
    socket_(SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK),
    channel_(std::make_unique<Channel>(loop, socket_.getFd())) {
    channel_->setOnWriteEvent([this] { handleWrite(); });

    initSocket();
}

void detail::Connector::start() {
    attemptToConnect_ = 0;
    const int sockfd = socket_.getFd();

    int res = socket_.connect(addr_);
    if (res == 0) {
        if (newConnectionCallback_) {
            state_ = State::CONNECTED;
            attemptToConnect_ = 0;
            socket_.setFd(-1); // 移交fd的管理权，避免一个fd调用两次close
            newConnectionCallback_(sockfd, addr_);
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

    // 其他错误
    retry();
}

void detail::Connector::stop() {
    channel_->disableAll();
    ::close(socket_.getFd());
    socket_.setFd(-1);
}

void detail::Connector::setRetryInterval(uint32_t retryIntervalMillis) {
    retryIntervalMillis_ = retryIntervalMillis;
}

void detail::Connector::setNewConnectionCallback(NewConnectionCallback newConnectionCallback) {
    newConnectionCallback_ = std::move(newConnectionCallback);
}

void detail::Connector::initSocket() const {
    socket_.setTcpNoDelay(true); // TODO: 测试有没有这个选项，延迟多大
    socket_.setKeepAlive(true);
}

void detail::Connector::handleWrite() {
    if (state_ != State::CONNECTING) {
        return;
    }

    int error = 0;
    socklen_t len = sizeof(error);
    const int sockfd = socket_.getFd();
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        LOG_ERROR("getsockopt failed: {}", strerror(errno));
        retry();
        return;
    }

    if (error) {
        LOG_ERROR("error: {}", error);
        retry();
        return;
    }

    channel_->disableAll();
    if (newConnectionCallback_) {
        state_ = State::CONNECTED;
        attemptToConnect_ = 0;
        socket_.setFd(-1); // 移交fd的管理权，避免一个fd调用两次close
        newConnectionCallback_(sockfd, addr_);
        return;
    }

    LOG_WARN("the connect callback is not set yet, will close new client by default");
}

void detail::Connector::retry() {
    channel_->disableAll();
    ::close(socket_.getFd());
    socket_.setFd(-1);
    state_ = State::DISCONNECTED;

    if (++attemptToConnect_ < MAX_RETRIES) {
        LOG_ERROR("attempt to connect failed, try #{}", attemptToConnect_);
        socket_ = Socket(SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK);
        channel_ = std::make_unique<Channel>(loop_, socket_.getFd());
        channel_->setOnWriteEvent([this] { handleWrite(); });

        loop_->runAfter([this] {
            start();
        }, std::chrono::milliseconds(retryIntervalMillis_));

        return;
    }

    LOG_ERROR("failed to connect to server");
}

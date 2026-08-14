//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/eventLoop.h>

enum class ChannelStatus {
    OUT_LOOP,
    IN_LOOP,
};

namespace Dream {
    class EventLoop;
}

class Channel {
public:
    static constexpr uint32_t EVENT_NONE = 0x0;
    static constexpr uint32_t EVENT_READ = 0x1;
    static constexpr uint32_t EVENT_PRI = 0x2;
    static constexpr uint32_t EVENT_WRITE = 0x4;
    static constexpr uint32_t EVENT_ERROR = 0x8;
    static constexpr uint32_t EVENT_HUP = 0x10;
    using Functor = std::function<void()>;

    Channel(Dream::EventLoop::Impl* loop, int fd) : loop_(loop), fd_(fd) {}

    [[nodiscard]] int getFd() const;
    [[nodiscard]] ChannelStatus getStatus() const;
    void setStatus(ChannelStatus status);

    [[nodiscard]] bool isNoneEvent() const;
    [[nodiscard]] bool isReading() const;
    [[nodiscard]] bool isWriting() const;

    void enableReading();
    void enableWriting();
    void disableReading();
    void disableWriting();
    [[nodiscard]] uint32_t getListenEvent() const;
    void update();

    void setActualEvent(uint32_t event);

    void setOnReadEvent(Functor func);
    void setOnWriteEvent(Functor func);
    void setOnCloseEvent(Functor func);
    void setOnErrorEvent(Functor func);

    void handleEvent() const;

private:
    Dream::EventLoop::Impl* loop_ = nullptr;
    int fd_ = -1;

    uint32_t listenEvent_{};                // channel自身关心的事件
    uint32_t actualEvent_{};                // 实际返回的事件
    ChannelStatus status_ = ChannelStatus::OUT_LOOP;

    Functor onReadEvent_;
    Functor onWriteEvent_;
    Functor onCloseEvent_;
    Functor onErrorEvent_;
};

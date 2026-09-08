//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <dataHandler.hpp>
#include <memoryPool.hpp>

#include <DreamNet/DreamNet.h>

#include <memory>

class StreamClient : public DataHandler {
public:
    StreamClient(Dream::EventLoop* loop, const Dream::Address& address, uint32_t streamID);
    virtual ~StreamClient();

    void connect();
    void disconnect() const;

private:
    uint32_t onMessage(Dream::TcpConnectionPtr conn, const Dream::Buffer& buffer);

private:
    Dream::EventLoop* loop_ = nullptr;
    uint32_t streamID_ = 0;
    MemoryPool pool_;

    std::unique_ptr<Dream::TcpClient> client_;
};

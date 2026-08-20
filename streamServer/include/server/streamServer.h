//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <DreamNet/DreamNet.h>
#include <dataHandler.hpp>
#include <dreamThread.hpp>
#include <concurrentQueue.hpp>
#include <structs.h>

#include <memory>

class StreamServer : public DataHandler, public DreamThread {
public:
    StreamServer(Dream::EventLoop* loop, const Dream::Address& address);

    void startServer(uint32_t threadCount) const;
    void stopServer() const;

    ////////////////DataHandler//////////////////////////
    void handle(void* data) override;
    ////////////////DataHandler//////////////////////////

protected:
    ////////////////DreamThread//////////////////////////
    void run(std::stop_token st) override;
    ////////////////DreamThread//////////////////////////
private:
    std::optional<Frame> getFrame();

private:
    Dream::EventLoop* loop_ = nullptr;
    std::unique_ptr<Dream::TcpServer> server_;

    ConcurrentQueue<Frame> audioQue_;
    ConcurrentQueue<Frame> videoQue_;
};
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
#include <camera/cameraCapturer.h>
#include <camera/audioCapturer.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>

class StreamServer : public DataHandler, public DreamThread {
public:
    StreamServer(Dream::EventLoop* loop, const Dream::Address& address,
                 std::unique_ptr<Camera> camera, std::unique_ptr<Audio> audio);
    ~StreamServer() override;

    void startServer(uint32_t threadCount);
    void stopServer();

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
    // 共享时钟基准与生产者（采集线程）。
    // baseTime_ 必须先于采集器构造；采集器声明在队列之前，
    // 保证析构时（逆序）队列先于采集器销毁，残留帧仍能安全归还内存池。
    std::chrono::time_point<std::chrono::steady_clock> baseTime_;
    CameraCapturer cameraCapturer_;
    AudioCapturer audioCapturer_;

    Dream::EventLoop* loop_ = nullptr;
    std::unique_ptr<Dream::TcpServer> server_;
    ConcurrentQueue<Frame> audioQue_;
    ConcurrentQueue<Frame> videoQue_;

    std::atomic<bool> started_{ false };
    std::atomic<bool> stopped_{ false };
};

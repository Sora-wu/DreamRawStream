//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <dreamThread.hpp>
#include <dataHandler.hpp>
#include <memoryPool.hpp>
#include <camera/h264Encoder.h>
#include <camera/camera.h>

#include <memory>

class CameraCapturer : public DreamThread, public DataHandler {
public:
    explicit CameraCapturer(std::chrono::time_point<std::chrono::steady_clock> baseTime) : baseTime_(baseTime) {}
    // 设置摄像头，这个函数应该在开启线程之前调用
    void setCamera(Camera* camera);

protected:
    void run(std::stop_token st) override;

private:
    std::chrono::time_point<std::chrono::steady_clock> baseTime_{};
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<H264Encoder> h264Encoder_;

    MemoryPool pool_;
};

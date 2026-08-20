//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <dreamThread.hpp>
#include <dataHandler.hpp>
#include <memoryPool.hpp>
#include <camera/fdkAACEncoder.h>
#include <camera/audio.h>

#include <memory>

class AudioCapturer : public DreamThread, public DataHandler {
public:
    explicit AudioCapturer(std::chrono::time_point<std::chrono::steady_clock> baseTime) : baseTime_(baseTime) {}
    // 设置音频，这个函数应该在开启线程之前调用
    void setAudio(Audio* audio);

protected:
    void run(std::stop_token st) override;

private:
    std::chrono::time_point<std::chrono::steady_clock> baseTime_{};
    std::unique_ptr<Audio> audio_;
    std::unique_ptr<FdkAACEncoder> aacEncoder_;

    MemoryPool pool_;
};

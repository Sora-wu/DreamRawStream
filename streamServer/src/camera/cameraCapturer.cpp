//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <camera/cameraCapturer.h>
#include <structs.h>

#include <chrono>
#include <cstring>
#include <print>
#include <cassert>

void CameraCapturer::setCamera(Camera* camera) {
    camera_.reset(camera);
}

void CameraCapturer::run(std::stop_token st) {
    auto[width, height] = camera_->getActualResolution();
    h264Encoder_ = std::make_unique<H264Encoder>(width, height);

    assert(camera_);
    camera_->startCapture();

    auto baseTime = std::chrono::steady_clock::now();
    bool isFirstFrame = true;

    while (!st.stop_requested()) {
        CameraFrame cameraFrame = camera_->getBuffer();
        if (!cameraFrame.data.empty()) {
            auto captureTime = std::chrono::steady_clock::now();
            if (isFirstFrame) {
                baseTime = captureTime;
                isFirstFrame = false;
            }

            const int64_t currentPTS = std::chrono::duration_cast<std::chrono::milliseconds>(captureTime - baseTime).count();
            std::span<char> encodeBuffer = h264Encoder_->encode(cameraFrame.data.data());
            if (!encodeBuffer.empty()) {
                char* buffer = pool_.allocate(encodeBuffer.size());
                memcpy(buffer, encodeBuffer.data(), encodeBuffer.size());
                PooledBuffer pooledBuffer{ &pool_, buffer, (uint32_t)encodeBuffer.size() };
                Frame frame{ FrameType::Video, std::move(pooledBuffer), currentPTS };
                handle(&frame);
            }
        }
    }

    camera_->stopCapture();
    std::println("CameraCapturer thread exit");
}

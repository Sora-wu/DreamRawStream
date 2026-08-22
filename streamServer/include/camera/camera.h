//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <cstdint>
#include <vector>
#include <span>

#include <linux/videodev2.h>
#include <sys/time.h>

struct CameraFrame {
    std::span<char> data;
    timeval timestamp;
};

struct CameraParam {
    uint32_t width = 640;
    uint32_t height = 480;
    uint32_t fps = 30;
};

class Camera {
public:
    Camera(const char* url, const CameraParam& param);
    ~Camera();

    void startCapture() const;
    void stopCapture() const;

    [[nodiscard]] CameraFrame getBuffer();
    std::pair<uint32_t, uint32_t> getActualResolution() const;

private:
    void open();
    void init();
    void initMmap();
    void uninit();
    void close();

private:
    struct Buffer {
        void* start;
        uint32_t length;
    };

    const char* url_{};
    const CameraParam& param_{};
    int fd_ = -1;

    v4l2_capability capability_{};
    v4l2_format format_{};

    std::vector<Buffer> buffers_;

    char* cameraBuffer_ = nullptr;
    uint32_t cameraBufferCapacity_ = 0;
};

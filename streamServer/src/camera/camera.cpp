//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <camera/camera.h>

#include <cstring>
#include <print>
#include <cerrno>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

namespace {
    int cameraIOCtl(int fd, int req, void *arg) {
        int r = -1;

        do {
            r = ioctl(fd, req, arg);
        } while (r < 0 && EINTR == errno);

        return r;
    }
}

Camera::Camera(const char* url, const CameraParam& param) : url_(url), param_(param) {
    open();
    init();
}

Camera::~Camera() {
    uninit();
    close();
}

void Camera::startCapture() const {
    for (size_t i = 0; i < buffers_.size(); ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (cameraIOCtl(fd_, VIDIOC_QBUF, &buf) < 0) {
            std::println(stderr, "VIDIOC_QBUF");
            return;
        }
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (cameraIOCtl(fd_,VIDIOC_STREAMON, &type) < 0) {
        std::println(stderr, "VIDIOC_STREAMON");
    }
}

void Camera::stopCapture() const {
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (cameraIOCtl(fd_,VIDIOC_STREAMOFF, &type) < 0) {
        std::println(stderr, "VIDIOC_STREAMOFF");
    }
}

CameraFrame Camera::getBuffer() {
    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    // 阻塞，直到摄像头拿到数据
    if (cameraIOCtl(fd_, VIDIOC_DQBUF, &buf) < 0) {
        return {};
    }

    const timeval timestamp = buf.timestamp;

    memcpy(cameraBuffer_, buffers_[buf.index].start, buf.length);
    if (cameraIOCtl(fd_, VIDIOC_QBUF, &buf) < 0) {
        memset(cameraBuffer_, 0, cameraBufferCapacity_);
        return{};
    }

    return { std::span(cameraBuffer_, buf.length), timestamp };
}

std::pair<uint32_t, uint32_t> Camera::getActualResolution() const {
    return { format_.fmt.pix.width, format_.fmt.pix.height };
}

void Camera::open() {
    struct stat st{};
    if (stat(url_, &st) < 0) {
        std::println(stderr, "Cannot identify '{}': {}, {}", url_, errno, std::strerror(errno));
        ::exit(1);
    }

    if (!S_ISCHR(st.st_mode)) {
        std::println(stderr, "{} is no device", url_);
        ::exit(1);
    }

    fd_ = ::open(url_, O_RDWR, 0);
    if (fd_ < 0) {
        std::println(stderr, "Cannot open '{}': {}, {}", url_, errno, std::strerror(errno));
        ::exit(1);
    }
}

void Camera::init() {
    if (cameraIOCtl(fd_,VIDIOC_QUERYCAP, &capability_) < 0) {
        if (EINVAL == errno) {
            std::println(stderr, "{} is no V4L2 device", url_);
        } else {
            std::println(stderr, "VIDIOC_QUERYCAP");
        }
        return;
    }

    if (!(capability_.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        std::println(stderr, "{} is no video capture device", url_);
        return;
    }

    if (!(capability_.capabilities & V4L2_CAP_STREAMING)) {
        std::println(stderr, "{} does not support streaming i/o", url_);
        return;
    }

    std::println("VIDIOC_QUERYCAP");
    std::println("the camera driver is {}", reinterpret_cast<const char*>(capability_.driver));
    std::println("the camera card is {}", reinterpret_cast<const char*>(capability_.card));
    std::println("the camera bus info is {}", reinterpret_cast<const char*>(capability_.bus_info));
    std::println("the version is {}", capability_.version);

    /* Select video input, video standard and tune here. */
    cropcap_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    format_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format_.fmt.pix.width = param_.width;
    format_.fmt.pix.height = param_.height;
    format_.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;    // yuv422
    format_.fmt.pix.field = V4L2_FIELD_INTERLACED;      // 隔行扫描

    if (cameraIOCtl(fd_,VIDIOC_S_FMT, &format_) < 0) {
        std::println(stderr, "VIDIOC_S_FMT");
        return;
    }

    uint32_t actualWidth  = format_.fmt.pix.width;
    uint32_t actualHeight = format_.fmt.pix.height;
    std::println("Actual resolution after S_FMT: {}x{}", actualWidth, actualHeight);

    // 交给硬件来控制帧率
    v4l2_streamparm streamParm{};
    streamParm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamParm.parm.capture.timeperframe.numerator = 1;
    streamParm.parm.capture.timeperframe.denominator = param_.fps;
    if (cameraIOCtl(fd_, VIDIOC_S_PARM, &streamParm) < 0) {
        // 注意：有些 USB 摄像头不支持任意帧率，如果设置失败，驱动会自动退回到一个默认值
        std::println(stderr, "VIDIOC_S_PARM (Failed to set FPS)");
    } else {
        std::println("Hardware FPS set to: {}/{}", streamParm.parm.capture.timeperframe.denominator,
            streamParm.parm.capture.timeperframe.numerator);
    }

    initMmap();
    cameraBufferCapacity_ = param_.width * param_.height * 4;           // 保险期间给4分量
    cameraBuffer_ = new char[cameraBufferCapacity_]{};
}

void Camera::initMmap() {
    v4l2_requestbuffers req{};
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    // 分配内存
    if (cameraIOCtl(fd_,VIDIOC_REQBUFS, &req) < 0) {
        if (EINVAL == errno) {
            std::println(stderr, "{} does not support memory mapping", url_);
        } else {
            std::println(stderr, "VIDIOC_REQBUFS");
        }
        return;
    }

    if (req.count < 2) {
        std::println(stderr, "Insufficient buffer memory on {}", url_);
        return;
    }

    buffers_.resize(req.count);

    for (size_t i = 0; i < req.count; ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (cameraIOCtl(fd_,VIDIOC_QUERYBUF, &buf) < 0) {
            std::println(stderr, "VIDIOC_QUERYBUF");
            return;
        }

        buffers_[i].length = buf.length;
        buffers_[i].start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset);
        if (MAP_FAILED == buffers_[i].start) {
            std::println(stderr, "mmap");
            return;
        }
    }
}

void Camera::uninit() {
    for (const auto& buf : buffers_) {
        if (munmap(buf.start, buf.length) < 0) {
            std::println(stderr, "munmap");
            return;
        }
    }

    buffers_.clear();

    if (cameraBuffer_) {
        delete[] cameraBuffer_;
        cameraBuffer_ = nullptr;
    }
}

void Camera::close() {
    if (::close(fd_) < 0) {
        std::println(stderr, "can not close camera!");
        ::exit(1);
    }

    fd_ = -1;
}

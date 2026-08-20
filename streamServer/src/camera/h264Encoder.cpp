//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <camera/h264Encoder.h>

#include <print>
#include <cstring>

H264Encoder::H264Encoder(uint32_t width, uint32_t height) {
    x264_param_default(&param_);
    x264_param_default_preset(&param_, "fast", "zerolatency");

    param_.i_width = width;
    param_.i_height = height;
    param_.rc.i_lookahead = 0;	//i帧向前缓冲区
    param_.i_fps_num = 15;
    param_.i_fps_den = 1;
    param_.b_annexb = 1;
    param_.i_keyint_max = 30;
    param_.i_keyint_min = 15;
    param_.i_bframe = 0;
    param_.b_repeat_headers = 1;
    param_.i_threads = 1;
    param_.i_slice_count = 1;
    param_.i_slice_count_max = 1;
    x264_param_apply_profile(&param_, "main");

    handler_ = x264_encoder_open(&param_);
    if (!handler_) {
        std::println(stderr, "x264_encoder_open error");
        ::exit(1);
    }

    x264_picture_alloc(&pic_, X264_CSP_I420, width, height);
    pic_.img.i_csp = X264_CSP_I420;
    pic_.img.i_plane = 3;

    encodeBufferCapacity_ = width * height * 4;
    encodeBuffer_ = new char[encodeBufferCapacity_]{};
}

H264Encoder::~H264Encoder() {
    x264_picture_clean(&pic_);
    x264_encoder_close(handler_);
}

std::span<char> H264Encoder::encode(const char* inBuffer) {
    memset(encodeBuffer_, 0, encodeBufferCapacity_);
    uint8_t* y = pic_.img.plane[0];
    uint8_t* u = pic_.img.plane[1];
    uint8_t* v = pic_.img.plane[2];

    // yuv422转yuv420
    uint32_t yIndex = 0;
    uint32_t uIndex = 0;
    uint32_t vIndex = 0;
    uint32_t yuv422Len = param_.i_width * param_.i_height * 2;
    for (uint32_t i = 0; i < yuv422Len; i += 2) {
        *(y + yIndex) = *(inBuffer + i);
        ++yIndex;
    }

    bool isU = true;
    for (uint32_t i = 0; i < param_.i_height; i += 2) {
        uint32_t uvIndex = i * param_.i_width * 2;      // yuv422 类似两个分量，所以需要*2
        // 因为交叉布局往往是y先，所以循环初始化时需要+1
        for (uint32_t j = uvIndex + 1; j < uvIndex + param_.i_width * 2; j += 2) {
            if (isU) {
                *(u + uIndex) = *(inBuffer + j);
                ++uIndex;
                isU = false;
            }
            else {
                *(v + vIndex) = *(inBuffer + j);
                ++vIndex;
                isU = true;
            }
        }
    }
    pic_.i_type = X264_TYPE_AUTO;
    pic_.i_pts = pts_++;

    int nalCount = -1;
    x264_picture_t picOut{};
    if (x264_encoder_encode(handler_, &nal_, &nalCount, &pic_, &picOut) < 0) {
        std::println(stderr, "x264_encoder_encode error,type:{:08x}!", pic_.img.i_csp);
        return {};
    }

    uint32_t res = 0;
    char* outBuffer = encodeBuffer_;
    for (uint32_t i = 0; i < nalCount; ++i) {
        memcpy(outBuffer, nal_[i].p_payload, nal_[i].i_payload);
        outBuffer += nal_[i].i_payload;
        res += nal_[i].i_payload;
    }

    return { encodeBuffer_, res };
}

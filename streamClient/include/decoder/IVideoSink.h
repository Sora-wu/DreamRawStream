//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <client/structs.h>

extern "C" {
struct AVFrame;
}

class IVideoSink {
public:
    virtual ~IVideoSink() = default;
    virtual void onVideoFrame(const VideoFrame& videoFrame) = 0;
};
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

    // 注意：IVideoSink未拥有videoFrame，如果需要异步操作，请自行拷贝
    virtual void onVideoFrame(const VideoFrame& videoFrame) = 0;
};
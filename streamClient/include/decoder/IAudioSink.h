//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <client/structs.h>

extern "C" {
struct AVFrame;
}

class IAudioSink {
public:
    virtual ~IAudioSink() = default;
    virtual void onAudioFrame(const AudioFrame& audioFrame) = 0;
};
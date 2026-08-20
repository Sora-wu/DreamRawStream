//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <camera/cameraCapturer.h>
#include <camera/audioCapturer.h>
#include <server/streamServer.h>

#include <print>

#include <DreamNet/DreamNet.h>

using namespace Dream;

namespace {
    constexpr uint32_t THREAD_COUNT = 16;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println(stderr, "Usage: {} <camera url>", argv[0]);
        return -1;
    }

    CameraParam param{};
    Camera* camera = new Camera(argv[1], param);

    auto baseTime = std::chrono::steady_clock::now();
    CameraCapturer* cameraCapturer = new CameraCapturer(baseTime);
    cameraCapturer->setCamera(camera);

    Audio* audio = new Audio;
    AudioCapturer* audioCapturer = new AudioCapturer(baseTime);
    audioCapturer->setAudio(audio);

    EventLoop loop;
    const Address adress{ 11451 };
    StreamServer* server = new StreamServer(&loop, adress);
    cameraCapturer->setNextHandler(server);
    audioCapturer->setNextHandler(server);
    server->startServer(THREAD_COUNT);

    cameraCapturer->start("capturerV thread");
    audioCapturer->start("capturerA thread");

    loop.loop();

    return 0;
}
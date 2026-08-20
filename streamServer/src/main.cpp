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

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println(stderr, "Usage: {} <camera url>", argv[0]);
        return -1;
    }

    CameraParam param{};
    Camera* camera = new Camera(argv[1], param);
    auto [width, height] = camera->getActualResolution();
    CameraCapturer* cameraCapturer = new CameraCapturer;
    cameraCapturer->setCamera(camera);

    Audio* audio = new Audio;
    AudioCapturer* audioCapturer = new AudioCapturer;
    audioCapturer->setAudio(audio);

    EventLoop loop;
    const Address adress{ 11451 };
    StreamServer* server = new StreamServer(&loop, adress);
    cameraCapturer->setNextHandler(server);
    audioCapturer->setNextHandler(server);

    cameraCapturer->start("capturerV thread");
    audioCapturer->start("capturerA thread");
    server->start("server");

    loop.loop();

    return 0;
}
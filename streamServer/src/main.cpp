//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <camera/camera.h>
#include <camera/audio.h>
#include <server/streamServer.h>
#include <DreamNet/DreamNet.h>

#include <print>
#include <memory>
#include <thread>
#include <csignal>

using namespace Dream;

namespace {
    constexpr uint32_t THREAD_COUNT = 16;

    void blockShutdownSignals(sigset_t& set) {
        sigemptyset(&set);
        sigaddset(&set, SIGINT);
        sigaddset(&set, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &set, nullptr);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println(stderr, "Usage: {} <camera url>", argv[0]);
        return -1;
    }

    sigset_t shutdownSignals{};
    blockShutdownSignals(shutdownSignals);

    CameraParam param{};
    auto camera = std::make_unique<Camera>(argv[1], param);
    auto audio = std::make_unique<Audio>();

    EventLoop loop;
    const Address address{ 11451 };
    StreamServer server(&loop, address, std::move(camera), std::move(audio));

    server.startServer(THREAD_COUNT);

    std::jthread signalWatcher([&shutdownSignals, &server](std::stop_token st) {
        while (!st.stop_requested()) {
            timespec timeout{ 0, 100'000'000 };  // 100ms
            siginfo_t info{};
            const int sig = sigtimedwait(&shutdownSignals, &info, &timeout);

            if (sig == SIGINT || sig == SIGTERM) {
                std::println("stop server");
                server.stopServer();
                return;
            }
            if (sig < 0 && errno != EAGAIN && errno != EINTR) {
                return;  // 意外错误，避免空转
            }
        }
    });

    loop.loop();

    return 0;
}

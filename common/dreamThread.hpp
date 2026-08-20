//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <thread>
#include <string>

class DreamThread {
public:
    DreamThread() = default;
    DreamThread(const DreamThread&) = delete;
    DreamThread& operator=(const DreamThread&) = delete;

    virtual ~DreamThread() {
        exit();
        wait();
    }

    bool start(const std::string& threadName = {}) {
        if (thread_.joinable()) {
            return false;
        }

        thread_ = std::jthread([this](std::stop_token st) {
            run(st);
        });

        if (!threadName.empty()) {
            setThreadName(threadName);
        }

        return true;
    }

    void exit() {
        thread_.request_stop();
    }

    void wait() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    static void msleep(uint32_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    static void sleep(uint32_t s) {
        std::this_thread::sleep_for(std::chrono::seconds(s));
    }

protected:
    virtual void run(std::stop_token st) = 0;

private:
    void setThreadName(const std::string& name) {
        auto handle = thread_.native_handle();
        std::string safeName = name.length() > 15 ? name.substr(0, 15) : name;

#ifdef PLATFORM_LINUX
        pthread_setname_np(handle, safeName.c_str());
#endif
    }

private:
    std::jthread thread_;
};
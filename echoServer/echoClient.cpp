//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/DreamNet.h>

#include <iostream>
#include <print>
#include <string>
#include <thread>

class EchoClient {
public:
    EchoClient(Dream::EventLoop* loop, const Dream::Address& address): loop_(loop), client_(loop, address) {
        client_.setConnectionCallback([this](Dream::TcpConnectionPtr conn){ onConnection(conn); });
        client_.setMessageCallback([this](Dream::TcpConnectionPtr conn, Dream::Buffer& buffer){ return onMessage(conn, buffer); });
    }

    void connect() {
        client_.connect();
        startInputLoop();
    }

    // 在事件循环退出后调用，回收输入线程。
    void stop() {
        if (inputThread_.joinable()) {
            inputThread_.join();
        }
    }

private:
    void onConnection(Dream::TcpConnectionPtr conn) {
        if (conn->isConnected()) {
            std::println("Connection established: {}", conn->getRemoteAddress().getIP());
            return;
        }

        std::println("Connection close: {}", conn->getRemoteAddress().getIP());
    }

    uint32_t onMessage(Dream::TcpConnectionPtr conn, Dream::Buffer& buffer) {
        std::string_view msg = buffer.getView();
        std::print("recv: {}", msg);
        return msg.size();
    }

    void startInputLoop() {
        std::println("type a line and press Enter to send, Ctrl+D to quit");

        inputThread_ = std::jthread([this] {
            std::string line;
            while (std::getline(std::cin, line)) {
                line.push_back('\n');
                loop_->runInLoop([this, line = std::move(line)] {
                    client_.send(line.data(), static_cast<uint32_t>(line.size()));
                });
            }

            loop_->runInLoop([this] { loop_->quit(); });
        });
    }

private:
    Dream::EventLoop* loop_ = nullptr;
    Dream::TcpClient client_;
    std::jthread inputThread_;
};

int main() {
    Dream::EventLoop loop;
    Dream::Address address{ 11451 };
    EchoClient client{ &loop, address };
    client.connect();

    loop.loop();
    client.stop();

    return 0;
}

//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/DreamNet.h>

#include <print>

class EchoClient {
public:
    EchoClient(Dream::EventLoop* loop, const Dream::Address& address): loop_(loop), client_(loop, address) {
        client_.setConnectionCallback([this](Dream::TcpConnection* conn){ onConnection(conn); });
        client_.setMessageCallback([this](Dream::TcpConnection* conn, Dream::Buffer& buffer){ onMessage(conn, buffer); });
    }

    void connect() {
        client_.connect();
    }

private:
    void onConnection(Dream::TcpConnection* conn) {
        if (conn->isConnected()) {
            std::println("Connection established: {}", conn->getRemoteAddress().getIP());
            return;
        }

        std::println("Connection close: {}", conn->getRemoteAddress().getIP());
    }

    void onMessage(Dream::TcpConnection* conn, Dream::Buffer& buffer) {
        std::string_view msg = buffer.view();
        std::print("recv: {}", msg);
    }

private:
    Dream::EventLoop* loop_ = nullptr;
    Dream::TcpClient client_;
};

int main() {
    Dream::EventLoop loop;
    Dream::Address address{ 11451 };
    EchoClient client{ &loop, address };
    client.connect();

    loop.loop();

    return 0;
}
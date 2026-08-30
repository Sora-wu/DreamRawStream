//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/DreamNet.h>

#include <print>

class EchoServer {
public:
    EchoServer(Dream::EventLoop* loop, const Dream::Address& address) : loop_(loop), server_(loop, address) {
        server_.setConnectionCallback([this](Dream::TcpConnectionPtr conn){ onConnection(conn); });
        server_.setMessageCallback([this](Dream::TcpConnectionPtr conn, Dream::Buffer& buffer){ return onMessage(conn, buffer); });
    }

    void start() {
        // server_.setThreadCount(2);
        server_.start();
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
        conn->send(buffer);

        return msg.size();
    }

private:
    Dream::EventLoop* loop_ = nullptr;
    Dream::TcpServer server_;
};

int main() {
    Dream::EventLoop loop;
    Dream::Address address{ 11451 };
    EchoServer server{ &loop, address };
    server.start();
    loop.loop();

    return 0;
}
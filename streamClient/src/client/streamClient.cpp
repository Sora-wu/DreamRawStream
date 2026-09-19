//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <client/streamClient.h>
#include <client/structs.h>

using namespace Dream;

StreamClient::StreamClient(EventLoop* loop, const Address& address, uint32_t streamID) :
    loop_(loop),
    streamID_(streamID),
    client_(std::make_unique<TcpClient>(loop_, address)) {
}

StreamClient::~StreamClient() {
    disconnect();
}

void StreamClient::connect() {
    client_->connect();
    client_->setMessageCallback([this](TcpConnectionPtr conn, const Buffer& buffer) {
        return onMessage(conn, buffer);
    });
}

void StreamClient::disconnect() const {
    client_->disconnect();
    client_->setMessageCallback(nullptr);
}

// 这里需要注意把消息包一次性全部都读出来，不然生产跟不上消费
// 导致延迟越来越大
uint32_t StreamClient::onMessage(TcpConnectionPtr conn, const Buffer& buffer) {
    uint32_t consumedTotal = 0;

    while (true) {
        if (buffer.readableSize() < consumedTotal + sizeof(FrameHeader)) {
            break; // 不足一个包头，等下一次数据
        }

        FrameHeader header{};
        memcpy(&header, buffer.peek().data() + consumedTotal, sizeof(FrameHeader));
        const uint32_t headerSize = sizeof(FrameHeader) + header.size;
        if (buffer.readableSize() < consumedTotal + headerSize) {
            break; // 不足一个完整帧，等下一次数据
        }

        char* payload = pool_.allocate(header.size);
        memcpy(payload, buffer.peek().data() + consumedTotal + sizeof(FrameHeader), header.size);

        DecodeFrame df{};
        df.streamID = streamID_;
        df.frame = Frame{(FrameType)header.type, PooledBuffer{&pool_, payload, header.size}, header.pts};
        handle(&df);

        consumedTotal += headerSize;
    }

    return consumedTotal; // handleRead 会一次性 consume 这么多
}

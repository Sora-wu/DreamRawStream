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
   client_->setMessageCallback([this](TcpConnectionPtr conn, const Buffer& buffer) { return onMessage(conn, buffer); });
}

void StreamClient::disconnect() const {
   client_->disconnect();
   client_->setMessageCallback(nullptr);
}

uint32_t StreamClient::onMessage(TcpConnectionPtr conn, const Buffer& buffer) {
   if (buffer.readableSize() < sizeof(FrameHeader)) {
      // 连包头的大小都没有，直接返回
      return 0;
   }

   FrameHeader header{};
   memcpy(&header, buffer.peek().data(), sizeof(FrameHeader));
   const uint32_t headerSize = sizeof(FrameHeader) + header.size;
   if (buffer.readableSize() < headerSize) {
      return 0;
   }

   // 取出payload
   char* payload = pool_.allocate(header.size);
   memcpy(payload, buffer.peek().data() + sizeof(FrameHeader), header.size);

   DecodeFrame df{};
   df.streamID = streamID_;
   df.frame = Frame{ (FrameType)header.type, PooledBuffer{ &pool_, payload, header.size }, header.pts };
   handle(&df);

   return headerSize;
}

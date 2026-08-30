//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <DreamNet/address.h>
#include <common/log.hpp>

#include <arpa/inet.h>

using namespace Dream;

Address::Address(uint16_t port, const std::string& ip) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) != 1) {
        LOG_FATAL("inet_pton failed, IP: {}", ip);
    }
}

std::string Address::getIP() const {
    char buf[64]{};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

uint16_t Address::getPort() const {
    return ntohs(addr_.sin_port);
}

void Address::setAddr(const sockaddr_in& addr) {
    addr_ = addr;
}

sockaddr_in Address::getAddr() const {
    return addr_;
}

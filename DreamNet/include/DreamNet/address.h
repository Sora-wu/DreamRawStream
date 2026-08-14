//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <functional>
#include <string>

#include <netinet/in.h>

namespace Dream {
    class Address {
    public:
        Address() = default;
        explicit Address(const sockaddr_in& addr) : addr_(addr) {}
        Address(uint16_t port, const std::string& ip = "127.0.0.1");

        [[nodiscard]] std::string getIP() const;
        [[nodiscard]] uint16_t getPort() const;
        void setAddr(const sockaddr_in& addr);
        [[nodiscard]] sockaddr_in getAddr() const;


    private:
        sockaddr_in addr_{};
    };
}
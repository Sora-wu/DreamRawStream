//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <string>
#include <cstdint>

#include <netinet/in.h>

namespace Dream {
    class Address {
    public:
        explicit Address(const sockaddr_in& addr) : addr_(addr) {}
        Address(uint16_t port, const std::string& ip = "127.0.0.1");

        [[nodiscard]] std::string getIP() const;
        [[nodiscard]] uint16_t getPort() const;
        [[nodiscard]] const sockaddr_in* getAddr() const;

    private:
        sockaddr_in addr_{};
    };
}
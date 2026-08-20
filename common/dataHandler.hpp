//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

class DataHandler {
public:
    virtual ~DataHandler() = default;

    virtual void handle(void* data) {
        if (nextHandler_) {
            nextHandler_->handle(data);
        }
    }

    void setNextHandler(DataHandler* handler) {
        nextHandler_ = handler;
    }

private:
    DataHandler* nextHandler_ = nullptr;
};
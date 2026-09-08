//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

class DataHandler {
public:
    virtual ~DataHandler() = default;

    void setNextHandler(DataHandler* handler) {
        nextHandler_ = handler;
    }

protected:
    virtual void handle(void* data) {
        if (nextHandler_) {
            nextHandler_->handle(data);
        }
    }

private:
    DataHandler* nextHandler_ = nullptr;
};
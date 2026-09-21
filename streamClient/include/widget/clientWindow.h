//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#ifndef DREAMRAWSTREAM_CLIENTWINDOW_H
#define DREAMRAWSTREAM_CLIENTWINDOW_H

#include <QWidget>
#include <map>

QT_BEGIN_NAMESPACE

namespace Ui {
    class ClientWindow;
}

QT_END_NAMESPACE

namespace Dream {
    class EventLoop;
}

class VideoWidget;
class DecodeScheduler;
class StreamClient;

class ClientWindow : public QWidget {
    Q_OBJECT

public:
    explicit ClientWindow(DecodeScheduler* scheduler, Dream::EventLoop* loop, QWidget* parent = nullptr);
    ~ClientWindow() override;

    VideoWidget* getVideoWidget(uint32_t index) const;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void onInputEditFinished();

private:
    struct Client {
        std::unique_ptr<StreamClient> streamClient;
        QString streamUrl;
    };

    Ui::ClientWindow* ui;
    QList<VideoWidget*> videoWidgets_; // 声明 videoWidgets 容器，用于存放 9 个视频窗口
    uint32_t selectedIndex_ = 0;

    DecodeScheduler* scheduler_ = nullptr;
    Dream::EventLoop* loop_ = nullptr;
    std::map<uint32_t, Client> clientMap_;
};

#endif //DREAMRAWSTREAM_CLIENTWINDOW_H
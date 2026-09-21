//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#ifndef DREAMRAWSTREAM_CLIENTWINDOW_H
#define DREAMRAWSTREAM_CLIENTWINDOW_H

#include <QWidget>
#include <QList>

QT_BEGIN_NAMESPACE

namespace Ui {
    class ClientWindow;
}

QT_END_NAMESPACE

class VideoWidget;
class DecodeScheduler;

class ClientWindow : public QWidget {
    Q_OBJECT

public:
    explicit ClientWindow(DecodeScheduler* scheduler, QWidget* parent = nullptr);
    ~ClientWindow() override;

    VideoWidget* getVideoWidget(uint32_t index) const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QString getStreamUrl(QWidget* widget);

private:
    Ui::ClientWindow* ui;
    QList<VideoWidget*> videoWidgets_; // 声明 videoWidgets 容器，用于存放 9 个视频窗口
    DecodeScheduler* scheduler_ = nullptr;
};

#endif //DREAMRAWSTREAM_CLIENTWINDOW_H
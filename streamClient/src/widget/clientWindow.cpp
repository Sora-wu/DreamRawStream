//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <widget/clientWindow.h>
#include <widget/videoWidget.h>
#include "ui_ClientWindow.h"
#include <QEvent>
#include <QStyle>

ClientWindow::ClientWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::ClientWindow) {
    ui->setupUi(this);

    // 将 9 个视频窗口存入数组，方便管理
    videoWidgets_ = { ui->video_1, ui->video_2, ui->video_3,
                     ui->video_4, ui->video_5, ui->video_6,
                     ui->video_7, ui->video_8, ui->video_9 };

    // 为每个视频窗口安装事件过滤器来捕获鼠标点击
    for (QWidget* w : videoWidgets_) {
        w->installEventFilter(this);
    }

    // 默认选中九宫格第一个视频窗口，方便后续只播放选中窗体的音频
    if (!videoWidgets_.isEmpty()) {
        videoWidgets_.first()->setSelected(true);
    }
}

ClientWindow::~ClientWindow() {
    delete ui;
}

VideoWidget* ClientWindow::getVideoWidget(uint32_t index) const {
    if (index < videoWidgets_.size()) {
        return videoWidgets_[index];
    }

    return nullptr;
}

bool ClientWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        VideoWidget* clickedWidget = qobject_cast<VideoWidget*>(watched);
        if (clickedWidget && videoWidgets_.contains(clickedWidget)) {

            // 1. 处理高亮状态
            for (VideoWidget* w : videoWidgets_) {
                w->setSelected(w == clickedWidget);
            }

            // 2. 判断是否播放，更新左侧输入框
            QString url = getStreamUrl(clickedWidget);
            ui->input->setText(url);

            return true; // 事件已处理
        }
    }
    // 继续交由基类处理其他事件
    return QWidget::eventFilter(watched, event);
}

QString ClientWindow::getStreamUrl(QWidget* widget) {
    // TODO: 在这里强转你的自定义视频类，并获取真实的 URL
    // 示例:
    // MyVideoWidget* video = qobject_cast<MyVideoWidget*>(widget);
    // if (video && video->isPlaying()) {
    //     return video->currentUrl();
    // }
    return "";
}
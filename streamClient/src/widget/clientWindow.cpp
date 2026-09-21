//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <widget/clientWindow.h>
#include <widget/videoWidget.h>
#include <client/decodeScheduler.h>
#include <client/streamClient.h>

#include "ui_ClientWindow.h"
#include <QEvent>
#include <QStyle>
#include <QRegularExpression>
#include <QHostAddress>

namespace {
    struct HostPortInfo {
        bool    valid    = false; // 是否解析成功
        QString host;             // IP 或主机名（IPv6 已去掉方括号）
        quint16 port     = 0;     // 端口
        bool    hostIsIp = false; // host 是否为字面量 IP

        QString toString() const
        {
            if (!valid)
                return QStringLiteral("<invalid>");
            return QStringLiteral("%1:%2 (isIp=%3)")
                    .arg(host).arg(port).arg(hostIsIp ? "true" : "false");
        }
    };

    /**
     * 解析 "ip:port" / "host:port" 形式的字符串
     * 支持：
     *   127.0.0.1:9999
     *   localhost:8808
     *   http://127.0.0.1:9999/api?x=1   （自动剥离 scheme / path / query）
     *   [::1]:8080                      （IPv6 需用方括号包裹）
     */
    HostPortInfo parseHostPort(const QString &url) {
        HostPortInfo info;

        QString s = url.trimmed();
        if (s.isEmpty())
            return info;

        // 1) 去掉 scheme，例如 http://、https://、tcp://
        static const QRegularExpression schemeRe(QStringLiteral("^[a-zA-Z][a-zA-Z0-9+.\\-]*://"));
        s.remove(schemeRe);

        // 2) 去掉 path / query / fragment
        const int cut = s.indexOf(QRegularExpression(QStringLiteral("[/?#]")));
        if (cut >= 0)
            s = s.left(cut);

        // 3) 匹配 host:port  （IPv6 用 [] 包裹）
        static const QRegularExpression re(
            QStringLiteral(R"(^(?:\[([^\[\]]+)\]|([^:\[\]]+)):(\d{1,5})$)"));

        const QRegularExpressionMatch m = re.match(s);
        if (!m.hasMatch())
            return info;

        const QString host =
            (m.captured(1).isEmpty() ? m.captured(2) : m.captured(1)).trimmed();
        if (host.isEmpty())
            return info;

        bool ok = false;
        const uint port = m.captured(3).toUInt(&ok);
        if (!ok || port == 0 || port > 65535)
            return info;

        info.host = host;
        info.port = static_cast<quint16>(port);

        // 4) 判断 host 是否是合法 IP
        QHostAddress addr;
        info.hostIsIp = addr.setAddress(host)
                        && addr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol;

        info.valid = true;
        return info;
    }

}

ClientWindow::ClientWindow(DecodeScheduler* scheduler, Dream::EventLoop* loop, QWidget* parent) :
    scheduler_(scheduler), loop_(loop), QWidget(parent), ui(new Ui::ClientWindow) {
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

    connect(ui->input, &QLineEdit::editingFinished, this, &ClientWindow::onInputEditFinished);
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
            for (uint32_t i = 0; i < videoWidgets_.size(); ++i) {
                VideoWidget* w = videoWidgets_[i];
                w->setSelected(w == clickedWidget);
                if (w == clickedWidget) {
                    selectedIndex_ = i;
                    scheduler_->setAudioStreamID(i);
                }
            }

            QString url = "";
            if (clientMap_.contains(selectedIndex_)) {
                url = clientMap_[selectedIndex_].streamUrl;
            }
            ui->input->setText(url);

            return true; // 事件已处理
        }
    }
    // 继续交由基类处理其他事件
    return QWidget::eventFilter(watched, event);
}

void ClientWindow::onInputEditFinished() {
    // 该窗口已经被是否打开过，如果被打开过直接返回
    if (clientMap_.contains(selectedIndex_)) {
        return;
    }

    QString url = ui->input->text();
    HostPortInfo info = parseHostPort(url);
    if (!info.valid) {
        return;
    }

    Dream::Address addr{ info.port, info.host.toStdString() };
    std::unique_ptr<StreamClient> client = std::make_unique<StreamClient>(loop_, addr, selectedIndex_);
    client->connect();
    client->setNextHandler(scheduler_);
    clientMap_.emplace(selectedIndex_, Client{ std::move(client), url });
}
